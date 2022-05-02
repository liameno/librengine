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

    void crawler::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        auto json_crawler = json["crawler"];

        this->user_agent = json_crawler["user_agent"].get<std::string>();

        std::string proxy_string = json_crawler["proxy"].get<std::string>();

        if (!proxy_string.empty()) {
            this->proxy = http::proxy{proxy_string};
        }

        this->load_page_timeout_s = json_crawler["load_page_timeout_s"].get<size_t>();
        this->update_time_site_info_s_after = json_crawler["update_time_site_info_s_after"].get<size_t>();
        this->delay_time_s = json_crawler["delay_time_s"].get<size_t>();
        this->max_recursive_deep = json_crawler["max_recursive_deep"].get<size_t>();
        this->max_pages_site = json_crawler["max_pages_site"].get<size_t>();
        this->max_page_symbols = json_crawler["max_page_symbols"].get<size_t>();
        this->max_robots_txt_symbols = json_crawler["max_robots_txt_symbols"].get<size_t>();
        this->is_one_site = json_crawler["is_one_site"].get<bool>();
        this->is_http_to_https = json_crawler["is_http_to_https"].get<bool>();
        this->is_check_robots_txt = json_crawler["is_check_robots_txt"].get<bool>();
    }

    std::string crawler::to_str() const {
        const std::string format = "UA={0}\nStartSiteUrl={1}\nProxy={2}\nMaxRecDeep={3}"
                                   "\nLPageTimeoutS={4}\nUpdateTimeSISAfter={5}\nDelayTimeS={6}\nMaxPagesS={7}\nMaxPageSym={8}"
                                   "\nMaxRobotsTSym={9}\nIsOneSite={10}\nIsHttpToHttps={11}\nIsCheckRobots={12}";
        return str::format(format, user_agent, start_site_url,
                           (proxy) ? proxy->compute_curl_format() : "null", max_recursive_deep,
                           load_page_timeout_s, update_time_site_info_s_after, delay_time_s, max_pages_site,
                           max_page_symbols, max_robots_txt_symbols,
                           is_one_site, is_check_robots_txt, is_check_robots_txt);
    }

    void cli::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        auto json_cli = json["cli"];

        this->mode = json_cli["mode"].get<size_t>();
    }

    std::string cli::to_str() const {
        const std::string format = "Mode={1}";
        return str::format(format, mode);
    }

    void website::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        auto json_website = json["website"];

        this->port = json_website["port"].get<size_t>();

        std::string proxy_string = json_website["proxy"].get<std::string>();

        if (!proxy_string.empty()) {
            this->proxy = http::proxy{proxy_string};
        }

        auto nodes = json_website["nodes"];

        for (auto node : nodes) {
            this->nodes.push_back(node_s{node["name"], node["url"]});
        }
    }

    std::string website::to_str() const {
        const std::string format = "Port={0}\nProxy={1}\nNodes={2}";
        return str::format(format, port, (proxy) ? proxy->compute_curl_format() : "null", nodes.size());
    }

    void db::load_from_file(const std::string &path) {
        const std::string content = helper::get_file_content(path);
        nlohmann::json json = nlohmann::json::parse(content, nullptr, true, true);
        auto json_db = json["db"];

        this->url = json_db["url"].get<std::string>();
        this->api_key = json_db["api_key"].get<std::string>();
    }

    std::string db::to_str() const {
        const std::string format = "Url={0}\nApiKey={1}";
        return str::format(format, url, api_key);
    }
}