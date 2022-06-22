#include "../include/json_generator.h"

#include "../../lib/include/helper.h"

using namespace librengine::helper;

namespace json_generator {
    std::optional<std::string>
    website_json(const std::string &title, const std::string &url, const std::string &host, const std::string &desc,
                 const bool &has_trackers) {
        nlohmann::json json;

        json["title"] = title;
        json["url"] = url;
        json["host"] = host;
        json["desc"] = desc;
        json["has_trackers"] = has_trackers;
        json["rating"] = 100; //def = 100
        json["date"] = compute_time();

        try {
            return json.dump();
        } catch (const nlohmann::detail::type_error &e) { //crawler trap
            return std::nullopt;
        }
    }

    std::optional<std::string> robots_txt_json(const std::string &body, const std::string &host) {
        nlohmann::json json;

        json["body"] = body;
        json["host"] = host;
        json["date"] = compute_time();

        try {
            return json.dump();
        } catch (const nlohmann::detail::type_error &e) { //crawler trap
            return std::nullopt;
        }
    }
}