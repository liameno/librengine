#include "third_party/httplib.h"

#include <optional>
#include <librengine/opensearch.h>
#include <librengine/third_party/json/json.hpp>
#include <librengine/str.h>
#include <librengine/str_impl.h>
#include <iostream>
#include <cstring>
#include <thread>

using namespace librengine;

namespace pages {
    struct search_result {
        std::string id;
        std::string title;
        std::string url;
        std::string desc;
        size_t rating{0};
    };

    void set_variables(std::string &page_src) {
        const std::string header_src = R"(<li><a href="/home">Home</a></li><li><a href="/node/info">Node Info</a></li><li><a href="https://github.com/liameno/librengine">Github</a></li>)";
        page_src = str::replace(page_src, "{HEADER_CONTENT}", header_src);
    }

    static std::string get_file_content(const std::string &path) {
        std::ifstream ifstream_(path);
        ifstream_.seekg(0, std::ios::end);

        std::string buffer;
        buffer.resize(ifstream_.tellg());
        ifstream_.seekg(0);
        ifstream_.read(const_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }
    static void update(const std::string &id, const std::string &field, const size_t &value, opensearch::client &client) {
        const auto path = opensearch::client::path_options("website/_doc/" + id + "/_update");
        const auto type = opensearch::client::request_type::POST;

        nlohmann::json json;
        json["doc"][field] = value;

        const auto response = client.custom_request(path, type, json.dump());
    }
    static void update(const std::string &id, const std::string &field, const std::string &value, opensearch::client &client) {
        const auto path = opensearch::client::path_options("website/_doc/" + id + "/_update");
        const auto type = opensearch::client::request_type::POST;

        nlohmann::json json;
        json["doc"][field] = value;

        const auto response = client.custom_request(path, type, json.dump());
    }
    static size_t get_number_field_value(const std::string &id, const std::string &field, opensearch::client &client) {
        const auto path = opensearch::client::path_options("website/_doc/" + id);
        const auto type = opensearch::client::request_type::GET;

        const auto response = client.custom_request(path, type);
        nlohmann::json result_json = nlohmann::json::parse(*response);
        const auto rating = result_json["_source"][field];

        if (rating.is_null()) return 0;
        if (rating > 0) return (size_t)rating;

        return 0;
    }
    static std::optional<std::vector<search_result>> search(const std::string &q, const size_t &s, opensearch::client &client) {
        const auto path = opensearch::client::path_options("website/_search");
        const auto type = opensearch::client::request_type::POST;

        nlohmann::json json;

        json["query"]["query_string"]["fields"] = {"url", "title", "desc"};
        json["query"]["query_string"]["query"] = q;
        json["size"] = 10;
        json["from"] = s;

        const auto response = client.custom_request(path, type, json.dump());
        nlohmann::json result_json = nlohmann::json::parse(*response);
        const auto value = result_json["hits"]["total"]["value"];

        if (value.is_null()) return std::nullopt;
        if (value < 0) return std::nullopt;

        const auto body = result_json["hits"]["hits"];
        if (body.is_null()) return std::nullopt;

        std::vector<search_result> results;
        results.reserve(value);

        for (int i = 0; i < value; ++i) {
            search_result result;

            try {
                result.id = result_json["hits"]["hits"][i]["_id"];
                result.title = result_json["hits"]["hits"][i]["_source"]["title"];
                result.url = result_json["hits"]["hits"][i]["_source"]["url"];
                result.desc = result_json["hits"]["hits"][i]["_source"]["desc"];
                result.rating = result_json["hits"]["hits"][i]["_source"]["rating"];
            } catch (const nlohmann::json::exception &e) {
                continue;
            }

            results.push_back(result);
        }

        return results;
    }
    static size_t get_field_count(const std::string &field, opensearch::client &client) {
        const auto path = opensearch::client::path_options("website/_search");
        const auto type = opensearch::client::request_type::POST;

        nlohmann::json json;

        json["aggs"]["host_uniq"]["terms"]["field"] = field;
        json["aggs"]["host_uniq"]["terms"]["size"] = 1;
        json["size"] = 0;

        const auto response = client.custom_request(path, type, json.dump());
        nlohmann::json result_json = nlohmann::json::parse(*response);
        const auto value = result_json["aggregations"]["host_uniq"]["sum_other_doc_count"];

        if (value.is_null()) return 0;
        if (value > 0) return (size_t)value + 1;

        return 0;
    }

    static void home(const httplib::Request &request, httplib::Response &response) {
        std::string page_src = get_file_content("../../frontend/src/index.html");

        set_variables(page_src);
        response.status = 200;
        response.set_content(page_src, "text/html");
    }
    static void search(const httplib::Request &request, httplib::Response &response, opensearch::client &client) {
        std::string page_src = get_file_content("../../frontend/src/search.html");
        const std::string query = request.get_param_value("q");
        auto s_ = request.get_param_value("s");
        const size_t start_index = (!s_.empty()) ? std::stoi(s_) : 0;

        const std::string center_result_src_format = "<div class=\"center_result\">"
                                                     "<div class=\"content\">"
                                                     "<a class=\"title\" href=\"{1}\">{0}</a>"
                                                     "<div class=\"url\">{1}</div>"
                                                     "<div class=\"description\">{2}</div>"
                                                     "</div>"
                                                     "<div class=\"rating_container\">"
                                                     "<div class=\"rating\">"
                                                     "<div class=\"counter\">{3}/100</div>"
                                                     "<a class=\"plus rating_button\" href=\"plus_rating?id={4}\"><i class=\"fa fa-arrow-up\"></i></a>"
                                                     "<a class=\"minus rating_button\" href=\"minus_rating?id={4}\"><i class=\"fa fa-arrow-down\"></i></a>"
                                                     "</div>"
                                                     "</div>"
                                                     "</div>";
        std::string center_results_src;
        const auto search_results = search(query, start_index, client);

        if (search_results) {
            for (const auto &result : *search_results) {
                std::string desc;
                const size_t max_size = 350;

                if (result.desc.length() > max_size) desc = result.desc.substr(0, max_size - 3) + "...";
                else desc = result.desc;

                center_results_src.append(str::format(center_result_src_format, result.title, result.url, desc, result.rating, result.id));
            }
        }

        page_src = str::replace(page_src, "{CENTER_RESULTS}", center_results_src);
        page_src = str::replace(page_src, "{QUERY}", query);
        page_src = str::replace(page_src, "{PREV_PAGE}", std::to_string((start_index >= 10) ? start_index - 10 : 0));
        page_src = str::replace(page_src, "{NEXT_PAGE}", std::to_string(start_index + 10));

        set_variables(page_src);
        response.status = 200;
        response.set_content(page_src, "text/html");
    }
    static void plus_rating(const httplib::Request &request, httplib::Response &response, opensearch::client &client) {
        const std::string id = request.get_param_value("id");
        const size_t rating = get_number_field_value(id, "rating", client);

        if (rating  < 100) {
            update(id, "rating", rating + 1, client);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        response.status = 301;
        response.set_redirect(request.headers.find("Referer")->second);
    }
    static void minus_rating(const httplib::Request &request, httplib::Response &response, opensearch::client &client) {
        const std::string id = request.get_param_value("id");
        const size_t rating = get_number_field_value(id, "rating", client);

        if (rating > 0) {
            update(id, "rating", rating - 1, client);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        response.status = 301;
        response.set_redirect(request.headers.find("Referer")->second);
    }
    static void node_info(const httplib::Request &request, httplib::Response &response, opensearch::client &client) {
        std::string page_src = get_file_content("../../frontend/src/node/info.html");

        page_src = str::replace(page_src, "{WEBSITES_COUNT}", std::to_string(get_field_count("host", client)));
        page_src = str::replace(page_src, "{PAGES_COUNT}", std::to_string(get_field_count("url", client)));

        set_variables(page_src);
        response.status = 301;
        response.set_content(page_src, "text/html");
    }
    static void not_found(const httplib::Request &request, httplib::Response &response) {
        response.status = 301;
        response.set_redirect("/home");
    }
}

int main(int argc, char **argv) {
    if (argc <= 1) { std::cout << "Usage: bin [port]\nExample: ./backend 8080" << std::endl; return 1; }
    int port;

    try {
        port = std::stoi(argv[1]);
    } catch (const std::exception &e) {
        std::cout << "Port parse error" << std::endl;
        return 2;
    }

    if (port == 0) {  std::cout << "Port == 0" << std::endl; return 3; }

    using namespace httplib;
    auto client = opensearch::client("http://localhost:9200");

    Server server;
    server.set_mount_point("/", "../../frontend/");

    server.Get("/home", pages::home);
    server.Get("/search", [&client](const httplib::Request &req, httplib::Response &res) { pages::search(req, res, client); });
    server.Get("/plus_rating", [&client](const httplib::Request &req, httplib::Response &res) { pages::plus_rating(req, res, client); });
    server.Get("/minus_rating", [&client](const httplib::Request &req, httplib::Response &res) { pages::minus_rating(req, res, client); });
    server.Get("/node/info", [&client](const httplib::Request &req, httplib::Response &res) { pages::node_info(req, res, client); });
    server.Get(".*", pages::not_found);

    std::cout << "The server was started at http://127.0.0.1:" << port << std::endl;
    server.listen("0.0.0.0", port);

    return 0;
}