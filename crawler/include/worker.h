#include <string>
#include <lexbor/html/html.h>
#include <optional>
#include <queue>

#include <librengine/config.h>
#include <librengine/typesense.h>

#ifndef WORKER_H
#define WORKER_H

using namespace librengine;

class worker {
private:
    struct url {
        std::string site_url;
        std::string owner_url;
    };
public:
    enum class result {
        added,
        disallowed_robots,
        disallowed_file_type,
        work_false,
        already_added,
        pages_limit,
        null_or_limit,
        empty,
        error,
    };
private:
    config::all config;
    bool is_work = false;
public:
    std::queue<url> queue;
public:
    std::optional<std::string> get_added_robots_txt(const std::string &host);
    size_t hints_count_added(const std::string &field, const std::string &value);

    librengine::http::request::result_s site(const librengine::http::url &url);
    std::optional<std::string> get_robots_txt(const librengine::http::url &url);

    bool is_allowed_in_robots(const std::string &body, const http::url &url) const;
    bool normalize_url(librengine::http::url &url, const std::optional<std::string> &owner_host = std::nullopt) const;
public:
    explicit worker(const librengine::config::all &config);
    void main_thread();
    result work(url &url_);
};

#endif
