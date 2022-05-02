#include "config.h"

namespace librengine::config {
    std::string helper::get_file_content(const std::string &path) {
        std::ifstream stream(path);
        std::string buffer;

        stream.seekg(0, std::ios::end);
        buffer.resize(stream.tellg());
        stream.seekg(0);
        stream.read(const_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }

    void global::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        auto json_global = json["global"];

        auto nodes = json_global["nodes"];

        for (auto node : nodes) {
            this->nodes.push_back(node_s{node["name"], node["url"]});
        }
    }

    void crawler::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        auto json_crawler = json["crawler"];

        user_agent = json_crawler["user_agent"].get<std::string>();

        std::string proxy_string = json_crawler["proxy"].get<std::string>();

        if (!proxy_string.empty()) {
            proxy = http::proxy{proxy_string};
        }

        load_page_timeout_s = json_crawler["load_page_timeout_s"].get<size_t>();
        update_time_site_info_s_after = json_crawler["update_time_site_info_s_after"].get<size_t>();
        delay_time_s = json_crawler["delay_time_s"].get<size_t>();
        max_recursive_deep = json_crawler["max_recursive_deep"].get<size_t>();
        max_pages_site = json_crawler["max_pages_site"].get<size_t>();
        max_page_symbols = json_crawler["max_page_symbols"].get<size_t>();
        max_robots_txt_symbols = json_crawler["max_robots_txt_symbols"].get<size_t>();
        is_one_site = json_crawler["is_one_site"].get<bool>();
        is_http_to_https = json_crawler["is_http_to_https"].get<bool>();
        is_check_robots_txt = json_crawler["is_check_robots_txt"].get<bool>();
    }

    void cli::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        auto json_cli = json["cli"];

        std::string proxy_string = json_cli["proxy"].get<std::string>();

        if (!proxy_string.empty()) {
            proxy = http::proxy{proxy_string};
        }
    }

    void website::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        auto json_website = json["website"];

        port = json_website["port"].get<size_t>();

        std::string proxy_string = json_website["proxy"].get<std::string>();

        if (!proxy_string.empty()) {
            proxy = http::proxy{proxy_string};
        }
    }

    void db::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        auto json_db = json["db"];

        url = json_db["url"].get<std::string>();
        api_key = json_db["api_key"].get<std::string>();

        websites = typesense(url, "websites", api_key);
        robots = typesense(url, "robots", api_key);
    }

    void all::load_from_file(const std::string &path) {
        global_.load_from_file(path);
        crawler_.load_from_file(path);
        cli_.load_from_file(path);
        website_.load_from_file(path);
        db_.load_from_file(path);
    }
}