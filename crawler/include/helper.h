#include <string>
#include <lexbor/html/html.h>
#include <optional>

#include <librengine/config.h>
#include <librengine/opensearch.h>
#include "librengine/http.h"

#ifndef HELPER_H
#define HELPER_H

namespace helper {
    inline size_t compute_time();

    std::optional<std::string> lxb_string_to_std(const lxb_char_t *s);
    lxb_char_t *std_string_to_lxb(const std::string &s);

    std::optional<lxb_html_document*> parse_html(const std::string &response);

    std::string compute_search_website_json(const std::string &field, const std::string &phrase, const librengine::config::crawler &current_config);
    std::string compute_search_robots_txt_json(const std::string &field, const std::string &phrase, const librengine::config::crawler &current_config);

    std::optional<std::string> compute_website_json(const std::string &title, const std::string &url, const std::string &host, const std::string &desc, const bool &has_ads, const bool &has_analytics);
    std::optional<std::string> compute_robots_txt_json(const std::string &body, const std::string &host);

    std::string get_desc(const std::string &attribute_name, const std::string &attribute_value, lxb_html_document *document);
    std::string compute_desc(const std::string &tag_name, lxb_html_document *document);

    std::optional<std::string> get_added_robots_txt(const std::string &host, const librengine::config::crawler &current_config, librengine::opensearch::client &opensearch_client);
    size_t hints_count_added(const std::string &field, const std::string &url, const librengine::config::crawler &current_config, librengine::opensearch::client &opensearch_client);

    librengine::http::request::result_s site(const librengine::http::url &url, const librengine::config::crawler &current_config);
    bool is_allowed_in_robots(const std::string &body, const std::string &url, const librengine::config::crawler &current_config);
    std::optional<std::string> get_robots_txt(const librengine::http::url &url, const librengine::config::crawler &current_config);
}

#endif