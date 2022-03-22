#include <string>
#include <lexbor/html/html.h>
#include <optional>

#include <librengine/config.h>
#include <librengine/opensearch.h>

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
    librengine::config::crawler current_config;
    librengine::opensearch::client opensearch_client;
    bool is_work = false;
public:
    bool normalize_url(librengine::http::url &url, const std::optional<std::string> &owner_host = std::nullopt) const;
public:
    worker(librengine::config::crawler config, librengine::opensearch::client opensearch_client);
    result main_thread(const std::string &site_url, int &deep, const std::optional<librengine::http::url> &owner_url = std::nullopt);
};

#endif
