#include "pages.h"

#include <optional>
#include <librengine/config.h>
#include <librengine/opensearch.h>
#include <librengine/logger.h>
#include <librengine/json.hpp>
#include <librengine/str.h>
#include <librengine/str_impl.h>
#include <librengine/http.h>
#include <cstring>
#include <thread>

#include "third_party/httplib.h"

#define DEBUG true //TODO: FALSE

void if_debug_print(const logger::type &type, const std::string &text, const std::string &ident) {
#if DEBUG
    logger::print(type, text, ident);
#endif
}

namespace backend::pages {
    using namespace librengine;

    void set_variables(std::string &page_src) {
        const std::string header_src = R"(<li><a href="/home">Home</a></li><li><a href="/node/info">Node Info</a></li><li><a href="https://github.com/liameno/librengine">Github</a></li>)";
        str::replace_ref(page_src, "{HEADER_CONTENT}", header_src);
    }

    void update(const std::string &id, const std::string &field, const size_t &value, opensearch::client &client) {
        const auto path = opensearch::client::path_options("website/_doc/" + id + "/_update");
        const auto type = opensearch::client::request_type::POST;

        nlohmann::json json;
        json["doc"][field] = value;

        const auto response = client.custom_request(path, type, json.dump());
    }
    void update(const std::string &id, const std::string &field, const std::string &value, opensearch::client &client) {
        const auto path = opensearch::client::path_options("website/_doc/" + id + "/_update");
        const auto type = opensearch::client::request_type::POST;

        nlohmann::json json;
        json["doc"][field] = value;

        const auto response = client.custom_request(path, type, json.dump());
    }
    size_t get_number_field_value(const std::string &id, const std::string &field, opensearch::client &client) {
        const auto path = opensearch::client::path_options("website/_doc/" + id);
        const auto type = opensearch::client::request_type::GET;

        const auto response = client.custom_request(path, type);
        nlohmann::json result_json = nlohmann::json::parse(*response);
        const auto rating = result_json["_source"][field];

        if (rating.is_null()) return 0;
        if (rating > 0) return (size_t)rating;

        return 0;
    }
    /*size_t get_last_added_website_date(opensearch::client &client) {
        const auto path = opensearch::client::path_options("website/_search");
        const auto type = opensearch::client::request_type::POST;

        nlohmann::json json;

        json["size"] = 1;
        json["sort"][0]["date"]["order"] = "desc";

        const auto response = client.custom_request(path, type, json.dump());
        nlohmann::json result_json = nlohmann::json::parse(*response);
        const auto value = result_json["hits"]["total"]["value"];

        if (value.is_null()) return std::nullopt;
        if (value < 0) return std::nullopt;

        const auto body = result_json["hits"]["hits"];
        if (body.is_null()) return std::nullopt;

        auto hit = body[0];

        size_t hit_date = hit["_source"]["date"];
        size_t current_date = time(nullptr);

        return current_date - hit_date;
    }*/
    std::optional<std::vector<search_result>> search(const std::string &q, const size_t &s, opensearch::client &client) {
        const auto path = opensearch::client::path_options("website/_search");
        const auto type = opensearch::client::request_type::POST;

        nlohmann::json json;

        json["query"]["query_string"]["fields"] = {"url", "title", "desc"};
        json["query"]["query_string"]["query"] = q;
        json["size"] = 10;
        json["from"] = s;

        const auto response = client.custom_request(path, type, json.dump());
        nlohmann::json result_json = nlohmann::json::parse(*response);

        const auto body = result_json["hits"]["hits"];
        if (body.is_null() || body.empty()) return std::nullopt;

        size_t value = body.size();

        std::vector<search_result> results;
        results.reserve(value);

        for (int i = 0; i < value; ++i) {
            search_result result;
            auto hit = body[i];

            try {
                result.id = hit["_id"];
                result.title = hit["_source"]["title"];
                result.url = hit["_source"]["url"];
                result.desc = hit["_source"]["desc"];
                result.rating = hit["_source"]["rating"];
                result.has_ads = hit["_source"]["has_ads"];
                result.has_analytics = hit["_source"]["has_analytics"];
            } catch (const nlohmann::json::exception &e) {
                continue;
            }

            results.push_back(result);
        }

        return results;
    }
    size_t get_field_count(const std::string &field, opensearch::client &client) {
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
        return (size_t)value + 1;
    }

    void home(const httplib::Request &request, httplib::Response &response, const config::website &config) {
        std::string page_src = config::helper::get_file_content("../../frontend/src/index.html");
        const std::string query = request.get_param_value("q");
        const std::string checkbox_src_format = "<span>"
                                                "<input class=\"checkbox\" id=\"{0}\" name=\"{0}\" type=\"checkbox\" checked=\"checked\">"
                                                "<label for=\"{0}\"><span>{0} [{1}]</span></label>"
                                                "</span>";
        std::string checkboxes_src;

        for (const auto &node : config.nodes) {
            checkboxes_src.append(str::format(checkbox_src_format, node.name, node.url));
        }

        str::replace_ref(page_src, "{QUERY}", query);
        str::replace_ref(page_src, "{CHECKBOXES}", checkboxes_src);

        set_variables(page_src);
        response.status = 200;
        response.set_content(page_src, "text/html");
    }
    void search(const httplib::Request &request, httplib::Response &response, opensearch::client &client, const config::website &config) {
        std::string page_src = config::helper::get_file_content("../../frontend/src/search.html");
        const std::string query = request.get_param_value("q");
        auto s_ = request.get_param_value("s");
        const size_t start_index = (!s_.empty()) ? std::stoi(s_) : 0;

        if_debug_print(logger::type::info, "q = " + query, request.path);
        if_debug_print(logger::type::info, "s = " + s_, request.path);

        const std::string center_result_src_format = "<div class=\"center_result\">"
                                                     "<div class=\"content\">"
                                                     "<a class=\"title\" href=\"{1}\">{0}<span><i class=\"fa fa-ad info_icon info_{6}\"></i><i class=\"fa fa-user-secret info_icon info_{7}\"></i></span></a>"
                                                     "<div class=\"url\">{1}</div>"
                                                     "<div class=\"description\">{2}</div>"
                                                     "</div>"
                                                     "<div class=\"rating_container\">"
                                                     "<div class=\"rating\">"
                                                     "<div class=\"counter\">{3}/200</div>"
                                                     "<a class=\"plus rating_button\" href=\"{5}/api/plus_rating?id={4}&redirect=1\"><i class=\"fa fa-arrow-up\"></i></a>"
                                                     "<a class=\"minus rating_button\" href=\"{5}f/api/minus_rating?id={4}&redirect=1\"><i class=\"fa fa-arrow-down\"></i></a>"
                                                     "</div>"
                                                     "</div>"
                                                     "</div>";
        const std::string search_param_src_format = R"(<input name="{0}" type="hidden" value="{1}">)";
        std::string center_results_src;
        std::string params_s = "?q=" + query + "&s=" + s_;
        std::string search_param_inputs;

        for (const auto &param : request.params) {
            if (param.first == "q" && param.first == "s") {
                continue;
            }

            for (const auto &node : config.nodes) {
                if (param.first != node.name) {
                    continue;
                }

                if_debug_print(logger::type::info, "param = " + param.first, request.path);
                params_s.append("&" + param.first + "=" + param.second);
                search_param_inputs.append(str::format(search_param_src_format, param.first, param.second));

                std::string search_url = str::format("{0}/api/search?q={1}&s={2}", node.url, str::replace(query, " ", "+"), s_);
                http::request request_(search_url);
                request_.perform();

                if_debug_print(logger::type::info, "search result code = " + std::to_string(request_.result.code), search_url);
                if_debug_print(logger::type::info, "search response is empty = " + std::to_string(request_.result.response->empty()), search_url);
                if (request_.result.code != 200 || request_.result.response->empty()) break;

                nlohmann::json json = nlohmann::json::parse(*request_.result.response);
                const auto size = json["count"];

                for (int i = 0; i < size; ++i) {
                    const auto result = json["results"][i];
                    const auto title = result["title"].get<std::string>();
                    const auto url = result["url"].get<std::string>();
                    const auto desc = result["desc"].get<std::string>();
                    const auto rating = result["rating"].get<size_t>();
                    const auto has_ads = result["has_ads"].get<bool>() ? "bad" : "good";
                    const auto has_analytics = result["has_analytics"].get<bool>() ? "bad" : "good";
                    const auto id = result["id"].get<std::string>();

                    std::string result_src = str::format(center_result_src_format, title, url, desc, rating, id, node.url, has_ads, has_analytics);
                    center_results_src.append(result_src);
                }

                break;
            }
        }

        std::string url = request.path + params_s;
        str::replace_ref(page_src, "{CENTER_RESULTS}", center_results_src);
        str::replace_ref(page_src, "{QUERY}", query);
        str::replace_ref(page_src, "{SEARCH}", search_param_inputs);
        str::replace_ref(page_src, "{PREV_PAGE}", str::replace(url, "&s=" + s_, "&s=" + std::to_string((start_index >= 10) ? start_index - 10 : 0)));
        str::replace_ref(page_src, "{NEXT_PAGE}", str::replace(url, "&s=" + s_, "&s=" + std::to_string(start_index + 10)));

        set_variables(page_src);
        response.status = 200;
        response.set_content(page_src, "text/html");
    }
    void node_info(const httplib::Request &request, httplib::Response &response, opensearch::client &client) {
        std::string page_src = config::helper::get_file_content("../../frontend/src/node/info.html");

        str::replace_ref(page_src, "{WEBSITES_COUNT}", std::to_string(get_field_count("host", client)));
        str::replace_ref(page_src, "{PAGES_COUNT}", std::to_string(get_field_count("url", client)));

        set_variables(page_src);
        response.status = 200;
        response.set_content(page_src, "text/html");
    }
    void node_admin_panel(const httplib::Request &request, httplib::Response &response, opensearch::client &client) {
        std::string page_src = config::helper::get_file_content("../../frontend/src/node/admin_panel/index.html");

        str::replace_ref(page_src, "{WEBSITES_COUNT}", std::to_string(get_field_count("host", client)));
        str::replace_ref(page_src, "{PAGES_COUNT}", std::to_string(get_field_count("url", client)));

        set_variables(page_src);
        response.status = 200;
        response.set_content(page_src, "text/html");
    }
    void api_plus_rating(const httplib::Request &request, httplib::Response &response, opensearch::client &client) {
        const std::string id = request.get_param_value("id");
        const std::string is_redirect = request.get_param_value("redirect");
        const size_t rating = get_number_field_value(id, "rating", client);

        if (rating < 200) {
            update(id, "rating", rating + 1, client);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (is_redirect == "1") {
            response.status = 301;
            const auto referer = request.headers.find("Referer")->second;
            const std::string redirect_url = (!referer.empty()) ? referer : "/";
            response.set_redirect(redirect_url);
        } else {
            response.status = 200;
        }
    }
    void api_minus_rating(const httplib::Request &request, httplib::Response &response, opensearch::client &client) {
        const std::string id = request.get_param_value("id");
        const std::string is_redirect = request.get_param_value("redirect");
        const size_t rating = get_number_field_value(id, "rating", client);

        if (rating > 0) {
            update(id, "rating", rating - 1, client);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (is_redirect == "1") {
            response.status = 301;
            const auto referer = request.headers.find("Referer")->second;
            const std::string redirect_url = (!referer.empty()) ? referer : "/";
            response.set_redirect(redirect_url);
        } else {
            response.status = 200;
        }
    }
    void api_search(const httplib::Request &request, httplib::Response &response, opensearch::client &client) {
        const std::string query = request.get_param_value("q");
        auto s_ = request.get_param_value("s");
        const size_t start_index = (!s_.empty()) ? std::stoi(s_) : 0;
        const auto search_results = search(query, start_index, client);
        nlohmann::json page_src;

        if (search_results) {
            auto sr_size = search_results->size();
            if_debug_print(logger::type::info, "search_results = " + sr_size, request.path);

            for (int i = 0; i < sr_size; ++i) {
                const auto &result = search_results->operator[](i);
                const size_t title_max_size = 55;
                const size_t desc_max_size  = 350;

                //remove html tags from text
                std::regex regex(R"(<\/?(\w+)(\s+\w+=(\w+|"[^"]*"|'[^']*'))*(( |)\/|)>)"); //<[^<>]+>
                std::string title = regex_replace(result.title, regex, "");
                std::string desc = regex_replace(result.desc, regex, "");

                if (title.length() > title_max_size) {
                    title = title.substr(0, title_max_size - 3) + "...";
                }
                if (desc.length() > desc_max_size) {
                    desc = desc.substr(0, desc_max_size - 3) + "...";
                }

                page_src["results"][i]["title"] = title;
                page_src["results"][i]["url"] = result.url;
                page_src["results"][i]["desc"] = desc;
                page_src["results"][i]["rating"] = result.rating;
                page_src["results"][i]["id"] = result.id;
                page_src["results"][i]["has_ads"] = result.has_ads;
                page_src["results"][i]["has_analytics"] = result.has_analytics;
            }

            page_src["count"] = search_results->size();
        } else {
            if_debug_print(logger::type::info, "search_results = null", request.path);
        }

        response.status = 200;
        response.set_content(page_src.dump(), "text/html");
    }
    void api_node_info(const httplib::Request &request, httplib::Response &response, opensearch::client &client) {
        nlohmann::json page_src;

        page_src["websites_count"] = get_field_count("host", client);
        page_src["pages_count"] = get_field_count("url", client);

        response.status = 200;
        response.set_content(page_src.dump(), "text/html");
    }
    void not_found(const httplib::Request &request, httplib::Response &response) {
        response.status = 301;
        response.set_redirect("/home");
    }
}