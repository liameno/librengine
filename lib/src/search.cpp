#include "search.h"

namespace librengine {
    search::search(const config::all &config) {
        this->config = config;

        for (const auto &node : config.global_.nodes) {
            http::request request_(node.url + "/api/get_rsa_public_key");
            if (!http::url(node.url).is_localhost()) request_.options.proxy = config.website_.proxy;
            request_.perform();

            rsa_public_keys.insert({node.url, request_.result.response.value_or("")});
        }
    }

    std::vector<search_result> search::local(const std::string &q, const size_t &p) {
        const auto response = config.db_.websites.search(q, "url,title,desc", {{"page", std::to_string(p)}});
        nlohmann::json result_json = nlohmann::json::parse(response);

        const auto body = result_json["hits"];
        if (body.is_null() || body.empty()) return {};

        size_t value = body.size();

        std::vector<search_result> results;
        results.reserve(value);

        for (int i = 0; i < value; ++i) {
            search_result result;
            auto hit = body[i];
            auto hit_doc = hit["document"];

            try {
                result.id = hit_doc["id"];
                result.title = hit_doc["title"];
                result.url = hit_doc["url"];
                result.desc = hit_doc["desc"];
                result.rating = hit_doc["rating"];
                result.has_ads = hit_doc["has_ads"];
                result.has_analytics = hit_doc["has_analytics"];
            } catch (const nlohmann::json::exception &e) {
                continue;
            }

            results.push_back(result);
        }

        return results;
    }

    std::vector<search_result> search::nodes(const std::string &q, const size_t &p) {
        return {};
    }
}