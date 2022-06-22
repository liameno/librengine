#include <string>
#include <lexbor/html/html.h>
#include <optional>
#include <queue>
#include <mutex>
#include <thread>

#include "../../lib/include/config.h"
#include "../../lib/include/typesense.h"
#include "../../lib/include/cache.h"

#ifndef WORKER_H
#define WORKER_H

using namespace librengine;

class worker {
public:
    struct url {
        std::string site_url;
        std::string owner_url;

        url() = default;
    };
    enum class result {
        added,
        disallowed_robots,
        disallowed_file_type,
        already_added,
        pages_limit,
        null_or_limit,
        empty,
        error,
    };
private:
    typedef std::queue<url> queue_t;
    typedef librengine::cache::lru<std::string, result> cache_t;
private:
    config::all config;
    bool is_work;

    std::vector<std::pair<std::shared_ptr<std::thread>, bool>> threads;

    std::shared_ptr<cache_t> cache_host;
    std::shared_ptr<cache_t> cache_url;
public:
    std::shared_ptr<queue_t> queue;
public:
    std::mutex threads_mutex;
    std::mutex queue_mutex;
    std::mutex cache_mutex;
public:
    std::optional<std::string> get_added_robots_txt(const std::string &host);
    size_t hints_count_added(const std::string &field, const std::string &value);

    librengine::http::request::result_s site(const librengine::http::url &url);
    std::optional<std::string> get_robots_txt(const librengine::http::url &url);

    bool is_allowed_in_robots(const std::string &body, const http::url &url) const;
    bool normalize_url(librengine::http::url &url, const std::optional<std::string> &owner_host = std::nullopt) const;
public:
    explicit worker(const librengine::config::all &config);

    void start_threads(const int &count, const bool &join = true);
    void stop_threads();

    void main_thread();
    result work(url &url_);
};

#endif
