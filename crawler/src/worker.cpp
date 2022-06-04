#include "../include/worker.h"

#include <lexbor/html/html.h>
#include <optional>
#include <thread>
#include <algorithm>

#include <librengine/http.h>
#include <librengine/str.h>
#include <librengine/logger.h>
#include <librengine/helper.h>
#include <librengine/cache.h>
#include <librengine/robots_txt.h>

#include "../include/json_generator.h"
#include "../include/html_helper.h"

#define DEBUG true //TODO: FALSE

void if_debug_print(const logger::type &type, const std::string &text, const std::string &ident) {
    #if DEBUG
    logger::print(type, text, ident);
    #endif
}

using namespace librengine;
using namespace librengine::helper;
using namespace json_generator;
using namespace html_helper;

std::optional<std::string> worker::get_added_robots_txt(const std::string &host) {
    const auto now = compute_time();
    auto filter_by = "date:>" + std::to_string(now - config.crawler_.update_time_site_info_s_after) +  " && date:<" + std::to_string(now);

    const auto search_response = config.db_.robots.search(host, "host", {{"filter_by", filter_by}, {"num_typos", "0"}});
    nlohmann::json result_json = nlohmann::json::parse(search_response);
    const auto found = result_json["found"];

    if (found > 0) {
        const auto body = result_json["hits"][0]["document"]["body"];
        if (body.is_null()) return std::nullopt;

        return body;
    }

    return std::nullopt;
}
size_t worker::hints_count_added(const std::string &field, const std::string &value) {
    const auto now = compute_time();
    auto filter_by = "date:>" + std::to_string(now - config.crawler_.update_time_site_info_s_after) +  " && date:<" + std::to_string(now);
    filter_by = http::url::escape(filter_by);

    const auto search_response = config.db_.websites.search(value, field, {{"filter_by", filter_by}, {"num_typos", "0"}});
    nlohmann::json result_json = nlohmann::json::parse(search_response);
    const auto found = result_json["found"];

    return found;
}

http::request::result_s worker::site(const http::url &url) {
    http::request request(url.text);

    request.options.timeout_s = config.crawler_.load_page_timeout_s;
    request.options.user_agent = config.crawler_.user_agent;
    request.options.proxy = config.crawler_.proxy;
    request.perform();

    return request.result;
}
std::optional<std::string> worker::get_robots_txt(const http::url &url) {
    http::url url_cp(url.text);
    url_cp.set(CURLUPART_PATH, "/robots.txt");
    url_cp.parse();

    http::request request(url_cp.text);
    request.options.timeout_s = config.crawler_.load_page_timeout_s;
    request.options.user_agent = config.crawler_.user_agent;
    request.options.proxy = config.crawler_.proxy;
    request.perform();

    if (request.result.code != 200) return std::nullopt;
    return request.result.response;
}

bool worker::is_allowed_in_robots(const std::string &body, const http::url &url) const {
    robots_txt robots(body);
    robots.parse();

    return robots.allowed(url, config.crawler_.user_agent);
}
bool worker::normalize_url(http::url &url, const std::optional<std::string> &owner_host) const {
    if (url.text.empty()) return false;
    if (url.text.size() < 3 && !owner_host) return false;

    if (starts_with(url.text, "//")) {
        url.text.insert(0, "http:");
        url.parse();
    }
    if (!url.host && owner_host && !owner_host->empty()) {
        http::url owner_url(*owner_host);
        owner_url.parse();

        owner_url.set(CURLUPART_QUERY, "");
        owner_url.set(CURLUPART_FRAGMENT, "");

        auto f_c = get_first(url.text);

        if (f_c == '.') {
            remove_first(url.text);
        } else if (f_c != '/') {
            if (get_last(owner_url.text) == '/') remove_last(owner_url.text);

            while(true) {
                const char c = get_last(owner_url.text);

                if (c == '/' || c == '\0') break;
                else remove_last(owner_url.text);
            }
        } else {
            owner_url.set(CURLUPART_PATH, "");
        }

        owner_url.compute_text();
        owner_url.parse();

        if (get_first(url.text) == '/' && get_last(owner_url.text) == '/') {
            remove_first(url.text);
        }

        url.text.insert(0, owner_url.text);
        url.compute_text();
        url.parse();
    }

    if (config.crawler_.is_http_to_https) {
        if (url.scheme && url.scheme == "http") {
            url.set(CURLUPART_SCHEME, "https");
        }
    }

    url.set(CURLUPART_QUERY, "");
    url.set(CURLUPART_FRAGMENT, "");

    while(true) {
        char c = get_last(url.text);

        if (c == '/' || c == '\0') {
            remove_last(url.text);
        } else {
            break;
        }
    }

    url.compute_text();
    url.parse();
    return true;
}

worker::worker(const config::all &config) {
    this->config = config;
    this->is_work = true;
}

void worker::main_thread() {
    cache::lru<std::string, result> cache_host(config.crawler_.max_lru_cache_size_host);
    cache::lru<std::string, result> cache_url(config.crawler_.max_lru_cache_size_url);

    while (!queue.empty()) {
        if (!is_work) break;

        auto url_ = queue.front();
        queue.pop();

        http::url site_url(to_lower_copy(url_.site_url));
        http::url owner_url(to_lower_copy(url_.owner_url));

        site_url.parse();
        owner_url.parse();

        if (!normalize_url(site_url, owner_url.text)) {
            if_debug_print(logger::type::error, "normalize url", site_url.text);
            cache_host.put(site_url.text, result::null_or_limit);
            continue;
        }
        if (!site_url.host) {
            if_debug_print(logger::type::error, "host = null", site_url.text);
            cache_host.put(site_url.text, result::null_or_limit);
            continue;
        }
        if (site_url.text == owner_url.text) {
            if_debug_print(logger::type::error, "url = owner", site_url.text);
            continue;
        }

        auto splited = split(*site_url.path, ".");
        auto file_type = (splited.size() <= 1) ? "" : splited.back();
        auto allowed_file_types = {"", "html", "html5", "php", "phtml"};

        if (std::find(allowed_file_types.begin(), allowed_file_types.end(), file_type) == allowed_file_types.end()) {
            if_debug_print(logger::type::error, "disallowed file type", site_url.text);
            cache_host.put(site_url.text, result::disallowed_file_type);
            continue;
        }

        url_.site_url = site_url.text;
        url_.owner_url = owner_url.text;

        auto cache = cache_host.get(*site_url.host);
        if (cache == nullptr) cache = cache_url.get(site_url.text);

        if (cache != nullptr) {
            if_debug_print(logger::type::error, "found in cache", site_url.text);
            continue;
        }

        if (hints_count_added("url", site_url.text) > 0) {
            if_debug_print(logger::type::error, "already added", site_url.text);
            cache_host.put(site_url.text, result::already_added);
            continue;
        }
        if (hints_count_added("host", *site_url.host) >= config.crawler_.max_pages_site) {
            if_debug_print(logger::type::error, "pages count >= limit", site_url.text);
            cache_host.put(*site_url.host, result::pages_limit);
            continue;
        }

        auto result = work(url_);

        if (result == result::work_false) {
            break;
        } else if (result == result::null_or_limit) {
            cache_host.put(site_url.text, result::null_or_limit);
        } else if (result == result::added || result == result::disallowed_robots) {
            //delay, after http request
            std::this_thread::sleep_for(std::chrono::seconds(config.crawler_.delay_time_s));
        }
    }
}
worker::result worker::work(url &url_) {
    if (!is_work) return result::work_false;

    http::url url(url_.site_url);
    http::url owner_url(url_.owner_url);

    url.parse();
    owner_url.parse();

    if (config.crawler_.is_check_robots_txt) {
        auto robots_txt_body = get_added_robots_txt(*url.host).value_or("");
        bool is_checked = true;

        if (robots_txt_body.empty()) {
            robots_txt_body = get_robots_txt(url).value_or("");
            auto robots_txt_body_length = robots_txt_body.length();

            if (robots_txt_body_length > 0 && robots_txt_body_length < config.crawler_.max_robots_txt_symbols) {
                const auto json = robots_txt_json(robots_txt_body, *url.host);
                if (!json) return result::null_or_limit;
                config.db_.robots.add(*json);
            } else {
                is_checked = false;
            }
        }

        if (is_checked && !is_allowed_in_robots(robots_txt_body, url)) {
            if_debug_print(logger::type::error, "disallowed robots.txt", url.text);
            return result::disallowed_robots;
        }
    }

    auto request_result = site(url);
    auto response = request_result.response;
    auto response_length = response->length();

    if_debug_print(logger::type::info, "response length = " + to_string(response_length), url.text);
    if_debug_print(logger::type::info, "response code = " + to_string(request_result.code), url.text);
    if_debug_print(logger::type::info, "curl code = " + to_string(request_result.curl_code), url.text);

    if (request_result.code != 200) {
        if_debug_print(logger::type::error, "code != 200", url.text);
        return result::null_or_limit;
    }
    if (!response || response_length < 1 || response_length >= config.crawler_.max_page_symbols) {
        if_debug_print(logger::type::error, "response = null | length < 1 | >= limit", url.text);
        return result::null_or_limit;
    }

    auto document = parse_html(*response);
    if (!document) return result::null_or_limit;
    auto body = lxb_dom_interface_node((*document)->body);
    if (body == nullptr) return result::null_or_limit;

    const std::string title = lxb_string_to_std(lxb_html_document_title((*document), nullptr)).value_or("");
    //const std::string content = lxb_string_to_std(lxb_dom_node_text_content(body, nullptr)).value_or("");
    std::string desc = get_desc("name", "description", *document); //meta tag

    if (desc.empty()) {
        desc = get_desc("http-equiv", "description", *document); //meta tag
    }
    if (desc.empty()) {
        desc.append(compute_desc("h1", *document)); //h1 tags
    }
    if (title.empty() && desc.empty()) {
        if_debug_print(logger::type::error, "title & desc are empty", url.text);
        return result::null_or_limit;
    }

    bool has_trackers = false;

    auto detect_trackers_strings = {
            //analytics
            "googletagmanager.com/", "yandex.ru/metrika/",
            "google-analytics.com/", "GoogleAnalyticsObject",
            "google-analytics.js",   "googletag",
            "googletagservices.com/",
            //ads
            "g.doubleclick.net ", "adservice.google.com",
            "amazon-adsystem.com/aax2/apstag.js", "yandex.ru/ads/system/context.js",
            R"("adServer":")", "googletag.pubads", "adsbygoogle.js",
            "googlesyndication.com/"
    };

    for (const auto &s : detect_trackers_strings) {
        if (contains(*response, s)) {
            has_trackers = true;
            break;
        }
    }

    const auto json = website_json(title, url.text, *url.host, desc, has_trackers);
    if (!json) return result::null_or_limit;

    config.db_.websites.add(*json);

    //print added url
    std::cout << logger::yellow << "[" << url.text << "]" << std::endl;

    auto collection = lxb_dom_collection_make(&(*document)->dom_document, 16);
    lxb_dom_elements_by_tag_name(lxb_dom_interface_element(body), collection, std_string_to_lxb("a"), 1);
    const auto a_length = collection->array.length;

    for (size_t i = 0; i < a_length; i++) {
        auto element = lxb_dom_collection_element(collection, i);
        const auto href_value = lxb_string_to_std(lxb_dom_element_get_attribute(element, std_string_to_lxb("href"), 4, nullptr));

        if (!href_value || *href_value == url.text || starts_with(*href_value, "#")) {
            continue;
        }

        http::url href_url(*href_value);
        href_url.parse();

        if (!starts_with(*href_value, "http")) {
            queue.push({href_url.text, url.text});
        } else {
            queue.push({*href_value, ""});
        }
    }

    if (a_length > 0) lxb_dom_collection_destroy(collection, true);

    lxb_html_document_destroy(*document);
    return result::added;
}