#include <string>
#include <lexbor/html/html.h>
#include <optional>

#include "config.h"
#include "../opensearch.h"

#ifndef WORKER_H
#define WORKER_H

namespace librengine::crawler {
    class worker {
    private:
        config current_config;
        opensearch::client opensearch_client;
        bool is_work = false;
    public:
        static inline size_t compute_time();

        static std::optional<std::string> lxb_string_to_std(const lxb_char_t *s);
        static lxb_char_t *std_string_to_lxb(const std::string &s);

        static std::optional<lxb_html_document*> parse_html(const std::string &response);
        bool normalize_url(http::url &url, const std::optional<std::string> &owner_host = std::nullopt) const;

        std::string compute_search_website_json(const std::string &field, const std::string &phrase) const;
        std::string compute_search_robots_txt_json(const std::string &field, const std::string &phrase) const;

        static std::optional<std::string> compute_website_json(const std::string &title, const std::string &url, const std::string &host, const std::string &content, const std::string &desc);
        static std::optional<std::string> compute_robots_txt_json(const std::string &body, const std::string &host);

        std::optional<std::string> get_added_robots_txt(const std::string &host);
        size_t hints_count_added(const std::string &field, const std::string &url);

        std::optional<std::string> site(const http::url &url);
        bool is_allowed_in_robots(const std::string &body, const std::string &url) const;
        std::optional<std::string> get_robots_txt(const http::url &url);

        static std::string compute_desc(lxb_html_document *document, lxb_dom_node *body);
    public:
        worker(config config, opensearch::client opensearch_client);
        void main_thread(const std::string &site_url, int &deep, const std::optional<std::string> &owner_host = std::nullopt);
    };
}

#endif
