#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>
#include <fstream>

#include "http.h"
#include "str_impl.h"
#include "json.hpp"

namespace librengine::config {
    namespace helper {
        std::string get_file_content(const std::string &path);
    }

    struct crawler {
        std::string user_agent;
        std::string start_site_url;

        std::optional<http::proxy> proxy;

        size_t load_page_timeout_s;
        size_t update_time_site_info_s_after;
        size_t delay_time_s;

        size_t max_recursive_deep;
        size_t max_pages_site;
        size_t max_page_symbols;
        size_t max_robots_txt_symbols;

        bool is_one_site;
        bool is_http_to_https;
        bool is_check_robots_txt;

        void load_from_file(const std::string &path);
        std::string to_str() const;
    };
    struct cli {
        std::string query;
        size_t start_index;
        size_t mode;

        void load_from_file(const std::string &path);
        std::string to_str() const;
    };
    struct website {
        struct node_s {
            std::string name;
            std::string url;
        };

        size_t port;
        std::optional<http::proxy> proxy;
        std::vector<node_s> nodes;

        void load_from_file(const std::string &path);
        std::string to_str() const;
    };
    struct db {
        std::string url;
        std::string api_key;

        void load_from_file(const std::string &path);
        std::string to_str() const;
    };
}

#endif