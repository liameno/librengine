#ifndef JSON_GENERATOR_H
#define JSON_GENERATOR_H

#include <optional>
#include <thread>
#include <algorithm>

namespace json_generator {
    std::optional<std::string> website_json(const std::string &title, const std::string &url, const std::string &host, const std::string &desc, const bool &has_trackers);
    std::optional<std::string> robots_txt_json(const std::string &body, const std::string &host);
};

#endif