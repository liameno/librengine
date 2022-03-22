#include <librengine/config.h>
#include <librengine/str.h>
#include <librengine/logger.h>

using namespace librengine;

struct search_result {
    std::string id;
    std::string title;
    std::string url;
    std::string desc;
    size_t rating{0};
};

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

static std::string search(opensearch::client &client, const config::cli &config) {
    std::string results;
    std::string result_format = "\t{0}\t/\t{1}\t/\t{2}\t/\t{3}\t/\t{4}\t/\t{5}";
    const std::string line = std::string(25, '=');

    if (config.mode == 0) {
        const auto search_results = search(config.query, config.start_index, client);

        if (search_results) {
            for (int i = 0; i < search_results->size(); ++i) {
                const auto &result = search_results->operator[](i);
                std::string desc;
                const size_t max_size = 350;

                if (result.desc.length() > max_size) desc = result.desc.substr(0, max_size - 3) + "...";
                else desc = result.desc;

                page_src["results"][i]["title"] = result.title;
                page_src["results"][i]["url"] = result.url;
                page_src["results"][i]["desc"] = desc;
                page_src["results"][i]["rating"] = result.rating;
                page_src["results"][i]["id"] = result.id;
            }

            page_src["count"] = search_results->size();
        }
    } else /*if (config.mode == 1)*/ {
        for (const auto &node : config.nodes) {
            results.append(line);
            results.append(node.name);
            results.append(line);

            http::request request_(node.url + "/api/search" + "?q=" + config.query + "&s=" + std::to_string(config.start_index));
            request_.perform();

            if (request_.result.code != 200 || request_.result.response->empty()) break;

            nlohmann::json json = nlohmann::json::parse(*request_.result.response);
            const auto size = json["count"];

            for (int i = 0; i < size; ++i) {
                const auto result = json["results"][i];
                const auto title = result["title"].get<std::string>();
                const auto url = result["url"].get<std::string>();
                const auto desc = result["desc"].get<std::string>();
                const auto rating = result["rating"].get<size_t>();
                const auto id = result["id"].get<std::string>();

                results.append(str::format(result_format, title, url, desc, rating, id, node.url));
            }

            results.append(line);
            results.append(std::string(node.name.length(), '='));
            results.append(line);
        }
    }

    return results;
}

int main(int argc, char **argv) {
    if (argc <= 2) {
        std::cout << "Usage: bin [query] [start_index] [config_path]\nExample: ./cli \"gnu\" 0 ../../config.json" << std::endl;
        return 1;
    }

    config::cli config;
    config.query = argv[1];
    config.start_index = std::stoi(argv[2]);
    config.load_from_file(argv[3]);

    std::cout   << logger::white << line << logger::green << "CFG" << logger::white << line << std::endl
                << logger::reset << config.to_str()  << std::endl
                << logger::white << line << "===" << logger::white << line << std::endl;

    return 0;
}