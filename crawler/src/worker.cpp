#include "../include/worker.h"

#include <lexbor/html/html.h>
#include <optional>
#include <thread>
#include <utility>

#include <librengine/http.h>
#include <librengine/str.h>
#include <librengine/logger.h>

#include "../include/helper.h"
#include "../third_party/rep-cpp/robots.h"

#define DEBUG true //TODO: FALSE

void if_debug_print(const logger::type &type, const std::string &text, const std::string &ident) {
    #if DEBUG
    logger::print(type, text, ident);
    #endif
}

using namespace helper;
using namespace librengine;

std::optional<std::string> worker::compute_website_json(const std::string &title, const std::string &url, const std::string &host, const std::string &desc, const bool &has_ads, const bool &has_analytics) {
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

std::string worker::get_desc(const std::string &attribute_name, const std::string &attribute_value, lxb_html_document *document) {
    auto collection = lxb_dom_collection_make(&(document)->dom_document, 16);
    lxb_dom_elements_by_attr(lxb_dom_interface_element(document->head), collection, std_string_to_lxb(attribute_name),
                             attribute_name.length(), std_string_to_lxb(attribute_value), attribute_value.length(), true);

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


std::optional<std::string> worker::get_added_robots_txt(const std::string &host) {
    const auto now = compute_time();
    auto filter_by = "date:>" + std::to_string(now - config.update_time_site_info_s_after) +  " && date:<" + std::to_string(now);

    const auto search_response = db_robots.search(host, "host", {{"filter_by", filter_by}});
    nlohmann::json result_json = nlohmann::json::parse(search_response);
    const auto value = result_json["found"];

    if (value.is_null()) return std::nullopt;
    if (value > 0) {
        const auto body = result_json["hits"][0]["document"]["body"];
        if (body.is_null()) return std::nullopt;

        return body;
    }

    return std::nullopt;
}
size_t worker::hints_count_added(const std::string &field, const std::string &url) {
    const auto now = compute_time();
    auto filter_by = "date:>" + std::to_string(now - config.update_time_site_info_s_after) +  " && date:<" + std::to_string(now);

    const auto search_response = db_website.search(url, "url", {{"filter_by", filter_by}});
    nlohmann::json result_json = nlohmann::json::parse(search_response);
    const auto value = result_json["found"];

    if (value.is_null()) return 0;
    if (value > 0) return value;

    return 0;
}

http::request::result_s worker::site(const http::url &url) {
    http::request request(url.text);

    request.options.timeout_s = config.load_page_timeout_s;
    request.options.user_agent = config.user_agent;
    request.options.proxy = config.proxy;
    request.perform();

    return request.result;
}
bool worker::is_allowed_in_robots(const std::string &body, const std::string &url) {
    Rep::Robots robots = Rep::Robots(body);
    return robots.allowed(url, config.user_agent);
}
std::optional<std::string> worker::get_robots_txt(const http::url &url) {
    http::url url_cp(url.text);
    url_cp.set(CURLUPART_PATH, "/robots.txt");
    url_cp.parse();

    http::request request(url_cp.text);
    request.options.timeout_s = config.load_page_timeout_s;
    request.options.user_agent = config.user_agent;
    request.options.proxy = config.proxy;
    request.perform();

    if (request.result.code != 200) return std::nullopt;
    return request.result.response;
}

bool worker::normalize_url(http::url &url, const std::optional<std::string> &owner_host) const {
    if (url.text.size() < 3 && !owner_host) {
        return false;
    }

    if (str::starts_with(url.text, "//")) {
        //insert protocol in url
        url.text.insert(0, "http:");
        url.parse();
    }
    if (!url.host && owner_host) {
        http::url owner_url(str::to_lower(*owner_host));
        owner_url.parse();

        owner_url.set(CURLUPART_QUERY, "");    //?param=value
        owner_url.set(CURLUPART_FRAGMENT, ""); //#id

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
            owner_url.set(CURLUPART_PATH, ""); // /a/b/c
        }

        owner_url.compute_text();
        owner_url.parse();

        if (str::get_first_char(url.text) == '/' && str::get_last_char(owner_url.text) == '/') {
            str::remove_first_char(url.text);
        }

        url.text.insert(0, owner_url.text);
        url.compute_text();
        url.parse();
    }

    if (this->config.is_http_to_https) {
        if (url.scheme && url.scheme == "http") {
            url.set(CURLUPART_SCHEME, "https"); //protocol
        }
    }

    url.set(CURLUPART_QUERY, "");    //?param=value
    url.set(CURLUPART_FRAGMENT, ""); //#id

    while(true) {
        char c = str::get_last_char(url.text);

        if (c == '/' || c == '\0') {
            str::remove_last_char(url.text);
        } else {
            break;
        }
    }

    url.compute_text();
    url.parse();
    return true;
}

worker::worker(config::crawler config, const config::db &db) : config(std::move(config)) {
    this->is_work = true;
    this->db_website = typesense(db.url, "websites", db.api_key);
    this->db_robots = typesense(db.url, "robots", db.api_key);
}

worker::result worker::main_thread(const std::string &site_url, int &deep, const std::optional<http::url> &owner_url) {
    if (!is_work) return result::work_false;

    http::url url(str::to_lower(site_url));
    url.parse();

    std::string owner_url_text = (owner_url) ? owner_url->text : "";

    if (!normalize_url(url, owner_url_text)) {
        if_debug_print(logger::type::error, "normalize url", url.text);
        return result::error;
    }
    if (!url.host) {
        if_debug_print(logger::type::error, "host == null", url.text);
        return result::null_or_limit;
    }
    if (url.text == owner_url_text) {
        if_debug_print(logger::type::error, "url == owner", url.text);
        return result::already_added;
    }
    if (config.is_one_site && owner_url && url.host != owner_url->host) {
        return result::already_added;
    }
    if (hints_count_added("url", url.text) > 0) {
        if_debug_print(logger::type::error, "already added", url.text);
        return result::already_added;
    }

    size_t pages_count = hints_count_added("host", *url.host);

    if (pages_count >= this->config.max_pages_site) {
        if_debug_print(logger::type::error, "pages count >= limit", url.text);
        return result::pages_limit;
    }

    if (this->config.is_check_robots_txt) {
        auto robots_txt_body = get_added_robots_txt(*url.host).value_or("");
        bool is_checked = true;

        if (robots_txt_body.empty()) {
            robots_txt_body = get_robots_txt(url).value_or("");
            auto robots_txt_body_length = robots_txt_body.length();

            if (robots_txt_body_length > 1 && robots_txt_body_length < this->config.max_robots_txt_symbols) {
                const auto json = compute_robots_txt_json(robots_txt_body, *url.host);
                if (!json) return result::null_or_limit;
                db_robots.add(*json);
            } else {
                is_checked = false;
            }
        }

        if (is_checked && !is_allowed_in_robots(robots_txt_body, url.text)) {
            return result::disallowed_robots;
        }
    }

    auto request_result = site(url);
    auto response = request_result.response;
    auto response_length = response->length();

    if_debug_print(logger::type::info, "response length = " + str::to_string(response_length), url.text);
    if_debug_print(logger::type::info, "response code = " + str::to_string(request_result.code), url.text);
    if_debug_print(logger::type::info, "curl code = " + str::to_string(request_result.curl_code), url.text);

    if (request_result.code != 200) {
        if_debug_print(logger::type::error, "code != 200", url.text);
        return result::null_or_limit;
    }
    if (!response || response_length < 1 || response_length >= this->config.max_page_symbols) {
        if_debug_print(logger::type::error, "response = null || length < 1 || >= limit", url.text);
        return result::null_or_limit;
    }

    auto document = parse_html(*response);
    if (!document) return result::null_or_limit;
    auto body = lxb_dom_interface_node((*document)->body);
    if (body == nullptr) return result::null_or_limit;

    const std::string title = lxb_string_to_std(lxb_html_document_title((*document), nullptr)).value_or("");
    //const std::string content = lxb_string_to_std(lxb_dom_node_text_content(body, nullptr)).value_or("");
    std::string desc = get_desc("name", "description", *document); //by meta tag

    if (desc.empty()) {
        desc = get_desc("http-equiv", "description", *document); //by meta tag
    }
    if (desc.empty()) {
        desc.append(compute_desc("h1", *document)); //from h1 tags
    }
    if (title.empty() && desc.empty()) {
        if_debug_print(logger::type::error, "title & desc are empty", url.text);
        return result::null_or_limit;
    }

    bool has_ads = false;
    bool has_analytics = false;

    const std::vector<std::string> detect_ads_strings = {
            "g.doubleclick.net ", "adservice.google.com",
            "amazon-adsystem.com/aax2/apstag.js", "yandex.ru/ads/system/context.js",
            R"("adServer":")", "googletag.pubads", "adsbygoogle.js",
            "googlesyndication.com/"
    };
    const std::vector<std::string> detect_analytics_strings = {
            "googletagmanager.com/", "yandex.ru/metrika/",
            "google-analytics.com/", "GoogleAnalyticsObject",
            "google-analytics.js",   "googletag",
            "googletagservices.com/"
    };

    for (const auto &s : detect_ads_strings) {
        if (str::contains(*response, s, false)) {
            has_ads = true;
            break;
        }
    }
    for (const auto &s : detect_analytics_strings) {
        if (str::contains(*response, s, false)) {
            has_analytics = true;
            break;
        }
    }

    const auto json = compute_website_json(title, url.text, *url.host, desc, has_ads, has_analytics);
    if (!json) return result::null_or_limit;

    db_website.add(*json);

    //print added url
    std::cout << logger::yellow << "[" << url.text << "]" << std::endl;

    if (deep < this->config.max_recursive_deep) {
        auto collection = lxb_dom_collection_make(&(*document)->dom_document, 16);
        lxb_dom_elements_by_tag_name(lxb_dom_interface_element(body), collection, std_string_to_lxb("a"), 1);
        const auto a_length = collection->array.length;
        std::vector<std::string> pages_limit_hosts;
        ++deep;

        for (size_t i = 0; i < a_length; i++) {
            auto element = lxb_dom_collection_element(collection, i);
            const auto href_value = lxb_string_to_std(lxb_dom_element_get_attribute(element, std_string_to_lxb("href"), 4, nullptr));

            if (!href_value && *href_value == url.text && str::starts_with(*href_value, "#")) {
                //skip fragment links
                continue;
            }

            http::url href_url(*href_value);
            href_url.parse();

            if (!href_url.host || str::contains(pages_limit_hosts, *href_url.host, true)) {
                //skip already added
                continue;
            }

            result result;

            if (!str::starts_with(*href_value, "http")) {
                result = main_thread(href_url.text, deep, url);
            } else {
                if (config.is_one_site && href_url.host != url.host) {
                    //skip other sites
                    continue;
                }

                result = main_thread(*href_value, deep);
            }

            if (result == result::work_false) {
                //exit from loop
                break;
            } else if (result == result::pages_limit) {
                pages_limit_hosts.push_back(*href_url.host);
            } else if (result == result::added || result == result::disallowed_robots) {
                //delay
                std::this_thread::sleep_for(std::chrono::seconds(this->config.delay_time_s));
            }
        }

        --deep;
        if (a_length > 0) lxb_dom_collection_destroy(collection, true);
    }

    lxb_html_document_destroy(*document);
    return result::added;
}