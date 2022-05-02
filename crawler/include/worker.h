#include <string>
#include <lexbor/html/html.h>
#include <optional>

#include <librengine/config.h>
#include <librengine/typesense.h>

#ifndef WORKER_H
#define WORKER_H

class worker {
public:
    enum class result {
        added,
        disallowed_robots,
        work_false,
        already_added,
        pages_limit,
        null_or_limit,
        empty,
        error,
    };
private:
    librengine::config::crawler config;
    librengine::typesense db_website;
    librengine::typesense db_robots;
    bool is_work = false;
public:
    std::optional<std::string> compute_website_json(const std::string &title, const std::string &url, const std::string &host, const std::string &desc, const bool &has_ads, const bool &has_analytics);
    std::optional<std::string> compute_robots_txt_json(const std::string &body, const std::string &host);

    std::string get_desc(const std::string &attribute_name, const std::string &attribute_value, lxb_html_document *document);
    std::string compute_desc(const std::string &tag_name, lxb_html_document *document);

    std::optional<std::string> get_added_robots_txt(const std::string &host);
    size_t hints_count_added(const std::string &field, const std::string &url);

    librengine::http::request::result_s site(const librengine::http::url &url);
    bool is_allowed_in_robots(const std::string &body, const std::string &url);
    std::optional<std::string> get_robots_txt(const librengine::http::url &url);

    bool normalize_url(librengine::http::url &url, const std::optional<std::string> &owner_host = std::nullopt) const;
public:
    worker(librengine::config::crawler config, const librengine::config::db &db);
    result main_thread(const std::string &site_url, int &deep, const std::optional<librengine::http::url> &owner_url = std::nullopt);
};

#endif
