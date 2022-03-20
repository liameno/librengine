#include <string>
#include <lexbor/html/html.h>
#include <optional>
#include <config.h>
#include <opensearch.h>

#include "../http.h"

#ifndef LIBRENGINE_HELPER_H
#define LIBRENGINE_HELPER_H

namespace librengine::crawler {
    class helper {
    public:
        static inline size_t compute_time();

        static std::optional<std::string> lxb_string_to_std(const lxb_char_t *s);
        static lxb_char_t *std_string_to_lxb(const std::string &s);

        static std::optional<lxb_html_document*> parse_html(const std::string &response);

        static std::string compute_search_website_json(const std::string &field, const std::string &phrase, const config::crawler &current_config);
        static std::string compute_search_robots_txt_json(const std::string &field, const std::string &phrase, const config::crawler &current_config);

        static std::optional<std::string> compute_website_json(const std::string &title, const std::string &url, const std::string &host, const std::string &desc, const bool &has_ads, const bool &has_analytics);
        static std::optional<std::string> compute_robots_txt_json(const std::string &body, const std::string &host);

        static std::string get_desc(lxb_html_document *document);
        static std::string compute_desc(const std::string &tag_name, lxb_html_document *document);

        static std::optional<std::string> get_added_robots_txt(const std::string &host, const config::crawler &current_config, opensearch::client &opensearch_client);
        static size_t hints_count_added(const std::string &field, const std::string &url, const config::crawler &current_config, opensearch::client &opensearch_client);

        static http::request::result_s site(const http::url &url, const config::crawler &current_config);
        static bool is_allowed_in_robots(const std::string &body, const std::string &url, const config::crawler &current_config);
        static std::optional<std::string> get_robots_txt(const http::url &url, const config::crawler &current_config);
    };
}

#endif