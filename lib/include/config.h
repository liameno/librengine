#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <fstream>

#include "http.h"
#include "str_impl.h"
#include "json.hpp"

namespace librengine::config {
    static std::string get_file_content(const std::string &path) {
        std::ifstream ifstream_(path);
        ifstream_.seekg(0, std::ios::end);

        std::string buffer;
        buffer.resize(ifstream_.tellg());
        ifstream_.seekg(0);
        ifstream_.read(const_cast<char *>(buffer.data()), buffer.size());

        return buffer;
    }

    struct crawler {
        std::string user_agent                = "librengine";
        std::string start_site_url            = "https://www.gnu.org";
        std::string opensearch_url            = "http://localhost:9200";      //without '/'

        std::optional<http::proxy> proxy      = std::nullopt; //socks5://127.0.0.1:9050

        size_t load_page_timeout_s            = 10;
        size_t update_time_site_info_s_after  = 864000;   //10 days
        size_t delay_time_s                   = 3;

        size_t max_recursive_deep             = 3;
        size_t max_pages_site                 = 300;
        size_t max_page_symbols               = 50000000;  //50 mb
        size_t max_robots_txt_symbols         = 3000;

        size_t is_http_to_https               = true;     //https://en.wikipedia.org/wiki/HTTPS
        size_t is_check_robots_txt            = true;     //https://en.wikipedia.org/wiki/Robots_exclusion_standard

        void load_from_file(const std::string &path = "config.json") {
            const std::string content = get_file_content(path);
            nlohmann::json json = nlohmann::json::parse(content);
            auto json_crawler = json["crawler"];

            this->user_agent = json_crawler["user_agent"].get<std::string>();
            this->opensearch_url = json_crawler["opensearch_url"].get<std::string>();
            this->proxy = http::proxy{json_crawler["proxy"].get<std::string>()};
            this->load_page_timeout_s = json_crawler["load_page_timeout_s"].get<size_t>();
            this->update_time_site_info_s_after = json_crawler["update_time_site_info_s_after"].get<size_t>();
            this->delay_time_s = json_crawler["delay_time_s"].get<size_t>();
            this->max_recursive_deep = json_crawler["max_recursive_deep"].get<size_t>();
            this->max_pages_site = json_crawler["max_pages_site"].get<size_t>();
            this->max_page_symbols = json_crawler["max_page_symbols"].get<size_t>();
            this->max_robots_txt_symbols = json_crawler["max_robots_txt_symbols"].get<size_t>();
            this->is_http_to_https = json_crawler["is_http_to_https"].get<bool>();
            this->is_check_robots_txt = json_crawler["is_check_robots_txt"].get<bool>();
        }

        std::string to_str() const {
            const std::string format = "UA={0}\nStartSiteUrl={1}\nOpenSearchUrl={2}\nProxy={3}\nMaxRecDeep={4}"
                                       "\nLPageTimeoutS={5}\nUpdateTimeSISAfter={6}\nDelayTimeS={7}\nMaxPagesS={8}\nMaxPageSym={9}"
                                       "\nMaxRobotsTSym={10}\nIsHttpToHttps={11}\nIsCheckRobots={12}";
            return str::format(format, user_agent, start_site_url, opensearch_url, (proxy) ? proxy->compute_curl_format() : "null", max_recursive_deep,
                               load_page_timeout_s, update_time_site_info_s_after, delay_time_s, max_pages_site, max_page_symbols, max_robots_txt_symbols,
                               is_check_robots_txt, is_check_robots_txt);
        }
    };
    struct website {
        struct node {
            std::string name;
            std::string url;
        };

        size_t port                         = 8080;
        std::optional<http::proxy> proxy    = std::nullopt; //socks5://127.0.0.1:9050
        std::vector<node> nodes             = {};

        static std::string get_file_content(const std::string &path) {
            std::ifstream ifstream_(path);
            ifstream_.seekg(0, std::ios::end);

            std::string buffer;
            buffer.resize(ifstream_.tellg());
            ifstream_.seekg(0);
            ifstream_.read(const_cast<char *>(buffer.data()), buffer.size());

            return buffer;
        }
        void load_from_file(const std::string &path = "config.json") {
            const std::string content = get_file_content(path);
            nlohmann::json json = nlohmann::json::parse(content);
            auto json_website = json["website"];

            this->port = json_website["port"].get<size_t>();
            this->proxy = http::proxy{json_website["proxy"].get<std::string>()};
            const auto size = json_website["nodes"].size();

            for (int i = 0; i < size; ++i) {
                node node;

                node.name = json_website["nodes"][i]["name"].get<std::string>();
                node.url = json_website["nodes"][i]["url"].get<std::string>();

                nodes.push_back(node);
            }
        }

        std::string to_str() const {
            const std::string format = "Port={0}\nNodes={1}";
            return str::format(format, port, nodes.size());
        }
    };
}

#endif