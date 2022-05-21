#include "search.h"

#include <lexbor/html/html.h>
#include <regex>

//      TODO: CONFIG        //
#define RSA_KEY_LENGTH      1024
#define TITLE_MAX_SIZE      55
#define DESC_MAX_SIZE       350
//      ============        //

namespace librengine {
    search::search(const config::all &config) {
        this->config = config;

        this->rsa = encryption::rsa();
        this->rsa.generate_keys(RSA_KEY_LENGTH);

        this->rsa_public_key = rsa.get_public_key_buffer();
        this->rsa_public_key_base64 = encryption::base64::easy_encode(rsa_public_key);
    }

    void search::init() {
        for (const auto &node : config.global_.nodes) {
            http::request request_(node.url + "/api/get_rsa_public_key");
            if (!http::url(node.url).is_localhost()) request_.options.proxy = config.website_.proxy;
            request_.perform();

            rsa_public_keys.insert({node.url, request_.result.response.value_or("")});
        }
    }


    void search::remove_html_tags(std::string &html) {
        std::regex regex(R"(<\/?(\w+)(\s+\w+=(\w+|"[^"]*"|'[^']*'))*(( |)\/|)>)"); //<[^<>]+>
        html = regex_replace(html, regex, "");
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
                result.node_url = "http://localhost";

                remove_html_tags(result.title);
                remove_html_tags(result.desc);

                if (result.title.length() > TITLE_MAX_SIZE) result.title = result.title.substr(0, TITLE_MAX_SIZE - 3) + "...";
                if (result.desc.length() > DESC_MAX_SIZE) result.desc = result.desc.substr(0, DESC_MAX_SIZE - 3) + "...";
            } catch (const nlohmann::json::exception &e) {
                continue;
            }

            results.push_back(result);
        }

        return results;
    }

    std::vector<search_result> search::nodes(std::string &query, const size_t &page, const bool &is_encryption_enabled, const std::string &encryption_key) {
        std::string page_ = std::to_string(page);

        if (encryption_key.find("END PUBLIC KEY") == -1) {
            return {};
        }

        if (is_encryption_enabled) {
            query = rsa.easy_private_decrypt(query);

            if (query.empty()) {
                return {};
            }
        }

        std::vector<search_result> results;

        for (const auto &node : config.global_.nodes) {
            std::string node_url_params;

            if (is_encryption_enabled) {
                encryption::rsa node_rsa;
                auto node_public_key = rsa_public_keys.find(node.url)->second;
                node_rsa.easy_read_public_key_buffer(node_public_key);
                auto encrypted_query = node_rsa.easy_public_encrypt(query);

                if (encrypted_query.empty()) {
                    return {};
                }

                node_url_params = str::format("?q={0}&p={1}&e=1&ek={2}", encrypted_query, page_, rsa_public_key_base64);
            } else {
                node_url_params = str::format("?q={0}&p={1}&e=0", query, page_);
            }

            http::request node_request(node.url + "/api/search" + node_url_params);
            if (!http::url(node.url).is_localhost()) node_request.options.proxy = config.website_.proxy;
            node_request.perform();

            const auto &response_code = node_request.result.code;
            const auto &response_text = node_request.result.response;

            if (response_code != 200 || response_text->empty()) break;
            auto node_response = *response_text;

            if (is_encryption_enabled) {
                node_response = rsa.easy_private_decrypt(node_response);

                if (node_response.empty()) {
                    return {};
                }
            }

            if (node_response == "null") continue;

            nlohmann::json json = nlohmann::json::parse(node_response);
            const auto size = json["count"];
            results.reserve(size);

            for (int i = 0; i < size; ++i) {
                search_result result;
                const auto &result_json = json["results"][i];

                result.id = result_json["id"];
                result.title = result_json["title"];
                result.url = result_json["url"];
                result.desc = result_json["desc"];
                result.rating = result_json["rating"];
                result.has_ads = result_json["has_ads"];
                result.has_analytics = result_json["has_analytics"];
                result.node_url = node.url;

                results.push_back(result);
            }
        }

        return results;
    }
}