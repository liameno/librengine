#include "crawler/worker.h"

#include <lexbor/html/html.h>
#include <optional>
#include <thread>
#include <utility>

#include "crawler/config.h"
#include "http.h"
#include "str.h"
#include "opensearch.h"
#include "../../third_party/json/json.hpp"
#include "../../third_party/rep-cpp/robots.h"

namespace librengine::crawler {
    size_t worker::compute_time() {
        return time(nullptr);
    }

    std::optional<std::string> worker::lxb_string_to_std(const lxb_char_t *s) {
        if (s == nullptr) return std::nullopt;
        return reinterpret_cast<const char *>(s);
    }
    lxb_char_t *worker::std_string_to_lxb(const std::string &s) {
        return (lxb_char_t *) s.c_str();
    }

    std::optional<lxb_html_document*> worker::parse_html(const std::string &response) {
        auto parser = lxb_html_parser_create();
        auto status = lxb_html_parser_init(parser);

        if (status != LXB_STATUS_OK) return std::nullopt;
        auto document = lxb_html_parse(parser, std_string_to_lxb(response), response.length() - 1);
        lxb_html_parser_destroy(parser);

        if (document == nullptr) return std::nullopt;
        return document;
    }
    bool worker::normalize_url(http::url &url, const std::optional<std::string> &owner_host) const {
        if (url.text.size() < 3 && !owner_host) return false;
        if (str::starts_with(url.text, "//")) {
            url.text.insert(0, "http:");
            url.parse();
        }
        if (!url.host && owner_host) {

            http::url owner_url(str::to_lower(*owner_host));
            owner_url.parse();

            owner_url.set(CURLUPART_QUERY, "");
            owner_url.set(CURLUPART_FRAGMENT, "");

            auto f_c = str::get_first_char(url.text);

            if (f_c == '.') {
                str::remove_first_char(url.text);
            } else if (f_c != '/') {
                while(true) {
                    const char c = str::get_last_char(owner_url.text);

                    if (c == '/' || c == '\0') {
                        break;
                    } else {
                        str::remove_last_char(owner_url.text);
                    }
                }
            } else {
                owner_url.set(CURLUPART_PATH, "");
            }

            url.parse();
            owner_url.parse();

            if (str::get_first_char(url.text) == '/' && str::get_last_char(owner_url.text) == '/') {
                str::remove_first_char(url.text);
            }

            url.text.insert(0, owner_url.text);
            url.parse();
        }

        if (this->current_config.is_http_to_https) {
            if (url.scheme && url.scheme == "http") {
                url.set(CURLUPART_SCHEME, "https");
            }
        }

        url.set(CURLUPART_QUERY, "");
        url.set(CURLUPART_FRAGMENT, "");

        while(true) {
            char c = str::get_last_char(url.text);

            if (c == '/' || c == '\0') {
                str::remove_last_char(url.text);
            } else {
                break;
            }
        }

        url.parse();
        return true;
    }

    std::string worker::compute_search_website_json(const std::string &field, const std::string &phrase) const {
        nlohmann::json json;
        const auto now = compute_time();

        json["query"]["bool"]["must"][0]["match"][field] = phrase;
        json["query"]["bool"]["must"][1]["range"]["date"]["gte"] = now - this->current_config.update_time_site_info_s_after;
        json["query"]["bool"]["must"][1]["range"]["date"]["lte"] = now;
        json["_source"] = false;

        return json.dump();
    }
    std::string worker::compute_search_robots_txt_json(const std::string &field, const std::string &phrase) const {
        nlohmann::json json;
        const auto now = compute_time();

        json["query"]["bool"]["must"][0]["match"][field] = phrase;
        json["query"]["bool"]["must"][1]["range"]["date"]["gte"] = now - this->current_config.update_time_site_info_s_after;
        json["query"]["bool"]["must"][1]["range"]["date"]["lte"] = now;

        return json.dump();
    }

    std::optional<std::string> worker::compute_website_json(const std::string &title, const std::string &url, const std::string &host, const std::string &content, const std::string &desc) {
        nlohmann::json json;

        json["title"] = title;
        json["url"] = url;
        json["host"] = host;
        json["content"] = content;
        json["desc"] = desc;
        json["date"] = compute_time();

        try {
            return json.dump();
        } catch (const nlohmann::detail::type_error &e) { //crawler trap
            return std::nullopt;
        }
    }
    std::optional<std::string> worker::compute_robots_txt_json(const std::string &body, const std::string &host) {
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

    std::optional<std::string> worker::get_added_robots_txt(const std::string &host) {
        const auto path = opensearch::client::path_options("robots_txt/_search");
        const auto type = opensearch::client::request_type::POST;
        const auto json = compute_search_robots_txt_json("host", host);
        const auto search_response = this->opensearch_client.custom_request(path, type, json);

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
    size_t worker::hints_count_added(const std::string &field, const std::string &url) {
        const auto path = opensearch::client::path_options("website/_search");
        const auto type = opensearch::client::request_type::POST;
        const auto json = compute_search_website_json(field, url);
        const auto search_response = this->opensearch_client.custom_request(path, type, json);

        if (!search_response) return false;
        nlohmann::json result_json = nlohmann::json::parse(*search_response);
        const auto value = result_json["hits"]["total"]["value"];

        if (value.is_null()) return 0;
        if (value > 0) {
            return value;
        }

        return 0;
    }

    std::optional<std::string> worker::site(const http::url &url) {
        http::request request(url.text);
        request.options.timeout_s = this->current_config.load_page_timeout_s;
        request.options.user_agent = this->current_config.user_agent;
        request.options.proxy = this->current_config.proxy;
        request.perform();

        if (request.result.code != 200) {
            return std::nullopt;
        }

        return request.result.response;
    }
    bool worker::is_allowed_in_robots(const std::string &body, const std::string &url) const {
        Rep::Robots robots = Rep::Robots(body);
        return robots.allowed(url, this->current_config.user_agent);
    }
    std::optional<std::string> worker::get_robots_txt(const http::url &url) {
        http::url url_cp(url.text);
        url_cp.set(CURLUPART_PATH, "/robots.txt");
        url_cp.parse();

        http::request request(url_cp.text);
        request.options.timeout_s = this->current_config.load_page_timeout_s;
        request.options.user_agent = this->current_config.user_agent;
        request.options.proxy = this->current_config.proxy;
        request.perform();

        if (request.result.code != 200) {
            return std::nullopt;
        }

        return request.result.response;
    }

    std::string worker::compute_desc(lxb_html_document *document, lxb_dom_node *body) {
        auto collection = lxb_dom_collection_make(&(document)->dom_document, 16);
        lxb_dom_elements_by_tag_name(lxb_dom_interface_element(body), collection, std_string_to_lxb("p"), 1);
        const auto p_length = collection->array.length;
        std::string desc;

        for (size_t i = 0; i < p_length; i++) {
            auto element = lxb_dom_collection_element(collection, i);
            const auto text = lxb_string_to_std(lxb_dom_node_text_content(lxb_dom_interface_node(element), nullptr)).value_or("");
            desc.append(text);
            desc.append("\n");
        }

        if (p_length > 0) lxb_dom_collection_destroy(collection, true);
        return desc;
    }

    worker::worker(config config, opensearch::client opensearch_client) : current_config(std::move(config)), opensearch_client(std::move(opensearch_client)) {
        this->is_work = true;
    }

    void worker::main_thread(const std::string &site_url, int &deep, const std::optional<std::string> &owner_host) {
        if (!is_work) return;

        http::url url(str::to_lower(site_url));
        url.parse();

        if (!normalize_url(url, owner_host.value_or(""))) return;
        if (url.text == owner_host) return;
        if (!url.host || hints_count_added("url", url.text) > 0) return;

        size_t pages_count = hints_count_added("host", *url.host);
        if (pages_count >= this->current_config.limit_pages_site) return;

        if (this->current_config.is_check_robots_txt) {
            auto robots_txt_body = get_added_robots_txt(*url.host).value_or("");
            bool is_check = true;

            if (robots_txt_body.empty()) {
                robots_txt_body = get_robots_txt(url).value_or("");

                if (robots_txt_body.length() < this->current_config.limit_robots_txt_symbols) {
                    const auto json = compute_robots_txt_json(robots_txt_body, *url.host);
                    if (!json) return;

                    const auto path = opensearch::client::path_options("robots_txt/_doc");
                    const auto type = opensearch::client::request_type::POST;

                    opensearch_client.custom_request(path, type, json);
                } else {
                    is_check = false;
                }
            }

            if (is_check && !is_allowed_in_robots(robots_txt_body, url.text)) return;
        }

        auto response = site(url);
        if (!response || response->length() >= this->current_config.limit_page_symbols) return;

        auto document = parse_html(*response);
        if (!document) return;
        auto body = lxb_dom_interface_node((*document)->body);
        if (body == nullptr) return;

        const std::string title = lxb_string_to_std(lxb_html_document_title((*document), nullptr)).value_or("#ERR#");
        const std::string content = lxb_string_to_std(lxb_dom_node_text_content(body, nullptr)).value_or("");
        const std::string desc = compute_desc(*document, body);

        const auto json = compute_website_json(title, url.text, *url.host, content, desc);
        if (!json) return;

        const auto path = opensearch::client::path_options("website/_doc");
        const auto type = opensearch::client::request_type::POST;

        opensearch_client.custom_request(path, type, json);
        std::cout << "[" << url.text << "]" << std::endl; //TODO: print

        if (deep < this->current_config.recursive_deep_max) {
            auto collection = lxb_dom_collection_make(&(*document)->dom_document, 16);
            lxb_dom_elements_by_tag_name(lxb_dom_interface_element(body), collection, std_string_to_lxb("a"), 1);
            const auto a_length = collection->array.length;
            ++deep;

            for (size_t i = 0; i < a_length; i++) {
                auto element = lxb_dom_collection_element(collection, i);
                const auto href_value = lxb_string_to_std(lxb_dom_element_get_attribute(element, std_string_to_lxb("href"), 4, nullptr));

                if (href_value && *href_value != url.text && !str::starts_with(*href_value, "#")) {
                    http::url href_url(*href_value);
                    href_url.parse();

                    std::this_thread::sleep_for(std::chrono::seconds(this->current_config.delay_time_s));

                    if (!str::starts_with(*href_value, "http")) {
                        main_thread(*href_value, deep, url.text);
                    } else {
                        main_thread(*href_value, deep);
                    }
                }
            }

            --deep;
            if (a_length > 0) lxb_dom_collection_destroy(collection, true);
        }

        lxb_html_document_destroy(*document);
    }
}