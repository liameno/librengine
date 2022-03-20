#include "crawler/helper.h"

#include <lexbor/html/html.h>
#include <optional>
#include <thread>
#include <utility>

#include "config.h"
#include "http.h"
#include "str.h"
#include "opensearch.h"
#include "json.hpp"
#include "../../third_party/rep-cpp/robots.h"

namespace librengine::crawler {
    size_t helper::compute_time() {
        return time(nullptr);
    }

    std::optional<std::string> helper::lxb_string_to_std(const lxb_char_t *s) {
        if (s == nullptr) return std::nullopt;
        return reinterpret_cast<const char *>(s);
    }
    lxb_char_t *helper::std_string_to_lxb(const std::string &s) {
        return (lxb_char_t *) s.c_str();
    }

    std::optional<lxb_html_document*> helper::parse_html(const std::string &response) {
        auto parser = lxb_html_parser_create();
        auto status = lxb_html_parser_init(parser);

        if (status != LXB_STATUS_OK) return std::nullopt;
        auto document = lxb_html_parse(parser, std_string_to_lxb(response), response.length() - 1);
        lxb_html_parser_destroy(parser);

        if (document == nullptr) return std::nullopt;
        return document;
    }

    std::string helper::compute_search_website_json(const std::string &field, const std::string &phrase, const config::crawler &current_config) {
        nlohmann::json json;
        const auto now = compute_time();

        json["query"]["bool"]["must"][0]["match"][field] = phrase;
        json["query"]["bool"]["must"][1]["range"]["date"]["gte"] = now - current_config.update_time_site_info_s_after;
        json["query"]["bool"]["must"][1]["range"]["date"]["lte"] = now;
        json["_source"] = false;

        return json.dump();
    }
    std::string helper::compute_search_robots_txt_json(const std::string &field, const std::string &phrase, const config::crawler &current_config) {
        nlohmann::json json;
        const auto now = compute_time();

        json["query"]["bool"]["must"][0]["match"][field] = phrase;
        json["query"]["bool"]["must"][1]["range"]["date"]["gte"] = now - current_config.update_time_site_info_s_after;
        json["query"]["bool"]["must"][1]["range"]["date"]["lte"] = now;

        return json.dump();
    }

    std::optional<std::string> helper::compute_website_json(const std::string &title, const std::string &url, const std::string &host, const std::string &desc, const bool &has_ads, const bool &has_analytics) {
        nlohmann::json json;

        json["title"] = title;
        json["url"] = url;
        json["host"] = host;
        json["desc"] = desc;
        json["has_ads"] = has_ads;
        json["has_analytics"] = has_analytics;
        json["rating"] = 100; //def = 100
        json["date"] = compute_time();

        try {
            return json.dump();
        } catch (const nlohmann::detail::type_error &e) { //crawler trap
            return std::nullopt;
        }
    }
    std::optional<std::string> helper::compute_robots_txt_json(const std::string &body, const std::string &host) {
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

    std::string helper::get_desc(lxb_html_document *document) {
        auto collection = lxb_dom_collection_make(&(document)->dom_document, 16);
        lxb_dom_elements_by_attr(lxb_dom_interface_element(document->head), collection, std_string_to_lxb("name"), 4, std_string_to_lxb("description"), 11, false);

        const auto c_length = collection->array.length;
        std::string desc;

        for (size_t i = 0; i < c_length; i++) {
            auto element = lxb_dom_collection_element(collection, i);
            const auto content = lxb_dom_element_get_attribute(element, std_string_to_lxb("content"), 7, nullptr);

            if (content != nullptr) {
                if (desc.length() > 500) break;
                desc.append(lxb_string_to_std(content).value_or(""));
                desc.append("\n");
            }
        }

        if (c_length > 0) lxb_dom_collection_destroy(collection, true);
        return desc;
    }
    std::string helper::compute_desc(const std::string &tag_name, lxb_html_document *document) {
        auto collection = lxb_dom_collection_make(&(document)->dom_document, 16);
        lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document->body), collection, std_string_to_lxb(tag_name), tag_name.length());

        const auto c_length = collection->array.length;
        std::string desc;

        for (size_t i = 0; i < c_length; i++) {
            if (desc.length() > 500) break;

            auto element = lxb_dom_collection_element(collection, i);
            const auto text = lxb_string_to_std(lxb_dom_node_text_content(lxb_dom_interface_node(element), nullptr)).value_or("");
            desc.append(text);
            desc.append("\n");
        }

        if (c_length > 0) lxb_dom_collection_destroy(collection, true);
        return desc;
    }
    
    
    std::optional<std::string> helper::get_added_robots_txt(const std::string &host, const config::crawler &current_config, opensearch::client &opensearch_client) {
        const auto path = opensearch::client::path_options("robots_txt/_search");
        const auto type = opensearch::client::request_type::POST;
        const auto json = helper::compute_search_robots_txt_json("host", host, current_config);
        const auto search_response = opensearch_client.custom_request(path, type, json);

        if (!search_response) return std::nullopt;
        nlohmann::json result_json = nlohmann::json::parse(*search_response);
        const auto value = result_json["hits"]["total"]["value"];

        if (value.is_null()) return std::nullopt;
        if (value > 0) {
            const auto body = result_json["hits"]["hits"][0]["_source"]["body"];
            if (body.is_null()) return std::nullopt;

            return body;
        }

        return std::nullopt;
    }
    size_t helper::hints_count_added(const std::string &field, const std::string &url, const config::crawler &current_config, opensearch::client &opensearch_client) {
        const auto path = opensearch::client::path_options("website/_search");
        const auto type = opensearch::client::request_type::POST;
        const auto json = helper::compute_search_website_json(field, url, current_config);
        const auto search_response = opensearch_client.custom_request(path, type, json);

        if (!search_response) return false;
        nlohmann::json result_json = nlohmann::json::parse(*search_response);
        const auto value = result_json["hits"]["total"]["value"];

        if (value.is_null()) return 0;
        if (value > 0) {
            return value;
        }

        return 0;
    }

    http::request::result_s helper::site(const http::url &url, const config::crawler &current_config) {
        http::request request(url.text);

        request.options.timeout_s = current_config.load_page_timeout_s;
        request.options.user_agent = current_config.user_agent;
        request.options.proxy = current_config.proxy;
        request.perform();

        return request.result;
    }
    bool helper::is_allowed_in_robots(const std::string &body, const std::string &url, const config::crawler &current_config) {
        Rep::Robots robots = Rep::Robots(body);
        return robots.allowed(url, current_config.user_agent);
    }
    std::optional<std::string> helper::get_robots_txt(const http::url &url, const config::crawler &current_config) {
        http::url url_cp(url.text);
        url_cp.set(CURLUPART_PATH, "/robots.txt");
        url_cp.parse();

        http::request request(url_cp.text);
        request.options.timeout_s = current_config.load_page_timeout_s;
        request.options.user_agent = current_config.user_agent;
        request.options.proxy = current_config.proxy;
        request.perform();

        if (request.result.code != 200) return std::nullopt;
        return request.result.response;
    }
}