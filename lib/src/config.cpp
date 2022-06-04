#include "config.h"

namespace librengine::config {
    std::string helper::get_file_content(const std::string &path) {
        std::ifstream stream(path);
        std::string buffer;

        stream.seekg(0, std::ios::end);
        buffer.resize(stream.tellg());
        stream.seekg(0);
        stream.read(buffer.data(), buffer.size());

        return buffer;
    }

    void global::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        json = json["global"];

        auto nodes = json["nodes"];

        for (auto node : nodes) {
            this->nodes.push_back(node_s{node["name"], node["url"]});
        }

        rsa_key_length = json["rsa_key_length"].get<size_t>();
        max_title_show_size = json["max_title_show_size"].get<size_t>();
        max_desc_show_size = json["max_desc_show_size"].get<size_t>();
    }

    void crawler::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        json = json["crawler"];

        user_agent = json["user_agent"].get<std::string>();

        std::string proxy_string = json["proxy"].get<std::string>();

        if (!proxy_string.empty()) proxy = http::proxy{proxy_string};

        load_page_timeout_s = json["load_page_timeout_s"].get<size_t>();
        update_time_site_info_s_after = json["update_time_site_info_s_after"].get<size_t>();
        delay_time_s = json["delay_time_s"].get<size_t>();
        max_pages_site = json["max_pages_site"].get<size_t>();
        max_page_symbols = json["max_page_symbols"].get<size_t>();
        max_robots_txt_symbols = json["max_robots_txt_symbols"].get<size_t>();
        max_lru_cache_size_host = json["max_lru_cache_size_host"].get<size_t>();
        max_lru_cache_size_url = json["max_lru_cache_size_url"].get<size_t>();
        is_http_to_https = json["is_http_to_https"].get<bool>();
        is_check_robots_txt = json["is_check_robots_txt"].get<bool>();
    }

    void cli::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        json = json["cli"];

        std::string proxy_string = json["proxy"].get<std::string>();

        if (!proxy_string.empty()) proxy = http::proxy{proxy_string};
    }

    void website::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        json = json["website"];

        port = json["port"].get<size_t>();

        std::string proxy_string = json["proxy"].get<std::string>();

        if (!proxy_string.empty()) proxy = http::proxy{proxy_string};
    }

    void db::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        json = json["db"];

        url = json["url"].get<std::string>();
        api_key = json["api_key"].get<std::string>();

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