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

#define DEBUG true //TODO: false
#define DEBUG_NORMALIZE false //TODO: false

void if_debug_print(const std::string &type, const std::string &text, const std::string &ident) {
#if DEBUG
    std::cout << "[" << librengine::str::to_upper(type) << "] " << text << " [" << ident << "]" << std::endl;
#endif
}
void if_debug_normalize_print(const std::string &type, const std::string &text, const std::string &ident) {
#if DEBUG_NORMALIZE
    std::cout << "[" << librengine::str::to_upper(type) << "] " << text << " [" << ident << "]" << std::endl;
#endif
}

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
        if_debug_normalize_print("info", "normalize url", url.text);
        if (str::starts_with(url.text, "//")) {
            if_debug_normalize_print("info", "url starts with //", url.text);
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
                if (str::get_last_char(owner_url.text) == '/') str::remove_last_char(owner_url.text);

                while(true) {
                    const char c = str::get_last_char(owner_url.text);

                    if (c == '/' || c == '\0') break;
                    else str::remove_last_char(owner_url.text);
                }
            } else {
                owner_url.set(CURLUPART_PATH, "");
            }

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
        if_debug_normalize_print("info", "normalized url", url.text);
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

        if (request.result.code != 200) return std::nullopt;
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

        if (request.result.code != 200) return std::nullopt;
        return request.result.response;
    }

    std::string worker::get_desc(lxb_html_document *document) {
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
    std::string worker::compute_desc(const std::string &tag_name, lxb_html_document *document) {
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

    worker::worker(config config, opensearch::client opensearch_client) : current_config(std::move(config)), opensearch_client(std::move(opensearch_client)) {
        this->is_work = true;
    }


    worker::result worker::main_thread(const std::string &site_url, int &deep, const std::optional<std::string> &owner_host) {
        if (!is_work) return result::work_false;

        http::url url(str::to_lower(site_url));
        url.parse();

        if (!normalize_url(url, owner_host.value_or(""))) { if_debug_print("error", "normalize url", url.text); return result::error; }
        if (url.text == owner_host) { if_debug_print("error", "url == owner", url.text); return result::already_added; }
        if (!url.host) { if_debug_print("error", "url host == null", url.text); return result::null_or_limit; }
        if (hints_count_added("url", url.text) > 0) { if_debug_print("error", "already added", url.text); return result::already_added; }

        size_t pages_count = hints_count_added("host", *url.host);
        if (pages_count >= this->current_config.limit_pages_site) { if_debug_print("error", "pages count >= limit", url.text);  return result::pages_limit; }

        if (this->current_config.is_check_robots_txt) {
            auto robots_txt_body = get_added_robots_txt(*url.host).value_or("");
            bool is_check = true;

            if (robots_txt_body.empty()) {
                robots_txt_body = get_robots_txt(url).value_or("");
                auto robots_txt_body_length = robots_txt_body.length();

                if (robots_txt_body_length > 1 && robots_txt_body_length < this->current_config.limit_robots_txt_symbols) {
                    const auto json = compute_robots_txt_json(robots_txt_body, *url.host);
                    if (!json) return result::null_or_limit;

                    const auto path = opensearch::client::path_options("robots_txt/_doc");
                    const auto type = opensearch::client::request_type::POST;

                    opensearch_client.custom_request(path, type, json);
                } else {
                    is_check = false;
                }
            }

            if (is_check && !is_allowed_in_robots(robots_txt_body, url.text)) return result::disallowed_robots;
        }

        auto response = site(url);
        auto response_length = response->length();
        if_debug_print("info", "response length = " + str::to_string(response_length), url.text);

        if (!response || response_length < 1 || response_length >= this->current_config.limit_page_symbols)
        { if_debug_print("error", "response = null || length < 1 || >= limit", url.text); return result::null_or_limit; }

        auto document = parse_html(*response);
        if (!document) return result::null_or_limit;
        auto body = lxb_dom_interface_node((*document)->body);
        if (body == nullptr) return result::null_or_limit;

        const std::string title = lxb_string_to_std(lxb_html_document_title((*document), nullptr)).value_or("#ERR#");
        //const std::string content = lxb_string_to_std(lxb_dom_node_text_content(body, nullptr)).value_or("");
        std::string desc = get_desc(*document); //by meta tag

        if (desc.empty()) {
            compute_desc("h1", *document);
            desc.append(compute_desc("p", *document));
        }

        const auto json = compute_website_json(title, url.text, *url.host, ""/*content*/, desc);
        if (!json) return result::null_or_limit;

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
                std::vector<std::string> pages_limit_hosts;

                if (href_value && *href_value != url.text && !str::starts_with(*href_value, "#")) {
                    http::url href_url(*href_value);
                    href_url.parse();

                    if (!href_url.host || str::contains(pages_limit_hosts, *href_url.host, true)) continue;

                    std::this_thread::sleep_for(std::chrono::seconds(this->current_config.delay_time_s));
                    result result;

                    if (!str::starts_with(*href_value, "http")) result = main_thread(href_url.text, deep, url.text);
                    else result = main_thread(*href_value, deep);

                    if (result == result::work_false) {
                        break;
                    } if (result == result::pages_limit) {
                        pages_limit_hosts.push_back(*href_url.host);
                    } if (result == result::added || result == result::disallowed_robots) {
                        std::this_thread::sleep_for(std::chrono::seconds(this->current_config.delay_time_s));
                    }
                }
            }

            --deep;
            if (a_length > 0) lxb_dom_collection_destroy(collection, true);
        }

        lxb_html_document_destroy(*document);
        return result::added;
    }
}
