#include "../include/worker.h"

#include <lexbor/html/html.h>
#include <optional>
#include <thread>
#include <algorithm>

#include "../../lib/include/logger.h"
#include "../../lib/include/helper.h"
#include "../../lib/include/robots_txt.h"

#include "../include/json_generator.h"
#include "../include/html_helper.h"

#define TRACKER_PATTERNS (std::vector { \
    "googletagmanager.com/", "yandex.ru/metrika/", \
    "google-analytics.com/", "GoogleAnalyticsObject", \
    "google-analytics.js",   "googletag", \
    "googletagservices.com/", "name=\"google", \
    "property=\"fb", "name=\"twitter" \
    \
    "g.doubleclick.net ", "adservice.google.com", \
    "amazon-adsystem.com/aax2/apstag.js", "yandex.ru/ads/system/context.js", \
    "\"adServer\":", "googletag.pubads", "adsbygoogle.js", \
    "googlesyndication.com/" \
}) //TODO: config
#define ALLOWED_CODES (std::vector {200}) //TODO: config
#define ALLOWED_FILE_TYPES (std::vector {"", "html", "html5", "php", "phtml"}) //TODO: config
#define DEBUG true //TODO: FALSE

#if DEBUG
std::mutex print_mutex;
#endif

void if_debug_print(const logger::type &type, const std::string &text, const std::string &ident) {
    #if DEBUG
    print_mutex.lock();
    logger::print(type, text, ident);
    print_mutex.unlock();
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

    const auto message = result_json["message"];

    if (!message.is_null()) {
        stop_threads();
        std::cout << "[DB_ERROR] " << message << std::endl;
        exit(1);
    }

    const auto found = result_json["found"];

    return found;
}

http::request::result_s worker::site(const http::url &url) {
    http::request request(url.text);

    request.options.timeout_s = config.crawler_.load_page_timeout_s;
    request.options.is_follow_location = true;
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
    this->is_work = false;

    this->queue = std::make_shared<queue_t>();
    this->cache_host = std::make_shared<cache_t>(config.crawler_.max_lru_cache_size_host);
    this->cache_url = std::make_shared<cache_t>(config.crawler_.max_lru_cache_size_url);
}

void worker::start_threads(const int &count, const bool &join) {
    this->is_work = true;
    threads.reserve(count);

    for (int i = 0; i < count; ++i) {
        auto t = std::make_shared<std::thread>(&worker::main_thread, this);
        threads.emplace_back(t, false);
    }

    if (join) {
        for (auto &t: threads) {
            t.first->join();
        }
    }
}
void worker::stop_threads() {
    this->is_work = false;
}

void worker::main_thread() {
    auto this_thread_id = std::this_thread::get_id();
    std::pair<std::shared_ptr<std::thread>, bool> *thread = nullptr;

    threads_mutex.lock();
    for (auto &t: threads) {
        if (t.first->get_id() == this_thread_id) {
            thread = &t;
        }
    }
    threads_mutex.unlock();

    if (thread == nullptr) {
        return;
    }

    while (true) {
        if (!is_work) break;

        queue_mutex.lock();
        bool is_empty = queue->empty();
        queue_mutex.unlock();

        if (is_empty) {
            bool is_working = false;

            threads_mutex.lock();
            for (auto &t: threads) {
                if (t.second) {
                    is_working = true;
                }
            }
            threads_mutex.unlock();

            if (!is_working) {
                break;
            } else {
                //wait for new urls
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }
        }

        thread->second = true;
        queue_mutex.lock();
        auto url_ = queue->front();
        queue->pop();
        queue_mutex.unlock();

        http::url site_url(to_lower_copy(url_.site_url));
        http::url owner_url(to_lower_copy(url_.owner_url));

        site_url.parse();
        owner_url.parse();

        if (!normalize_url(site_url, owner_url.text)) {
            if_debug_print(logger::type::error, "normalize url", site_url.text);
            cache_host->put(site_url.text, result::null_or_limit);
            thread->second = false;
            continue;
        }
        if (!site_url.host) {
            if_debug_print(logger::type::error, "host = null", site_url.text);
            cache_host->put(site_url.text, result::null_or_limit);
            thread->second = false;
            continue;
        }
        if (site_url.text == owner_url.text) {
            if_debug_print(logger::type::error, "url = owner", site_url.text);
            thread->second = false;
            continue;
        }

        auto splited = split(*site_url.path, ".");
        auto file_type = (splited.size() <= 1) ? "" : splited.back();

        if (std::find(ALLOWED_FILE_TYPES.begin(), ALLOWED_FILE_TYPES.end(), file_type) == ALLOWED_FILE_TYPES.end()) {
            if_debug_print(logger::type::error, "disallowed file type", site_url.text);
            cache_mutex.lock();
            cache_host->put(site_url.text, result::disallowed_file_type);
            cache_mutex.unlock();
            thread->second = false;
            continue;
        }

        url_.site_url = site_url.text;
        url_.owner_url = owner_url.text;

        cache_mutex.lock();
        auto cache = cache_host->get(site_url.host.value());
        if (cache == nullptr) cache = cache_url->get(site_url.text);
        cache_mutex.unlock();

        if (cache != nullptr) {
            //if_debug_print(logger::type::error, "found in cache", site_url.text);
            thread->second = false;
            continue;
        }

        if (hints_count_added("url", site_url.text) > 0) {
            if_debug_print(logger::type::error, "already added", site_url.text);
            cache_mutex.lock();
            cache_host->put(site_url.text, result::already_added);
            cache_mutex.unlock();
            thread->second = false;
            continue;
        }
        if (hints_count_added("host", site_url.host.value()) >= config.crawler_.max_pages_site) {
            if_debug_print(logger::type::error, "pages count >= limit", site_url.text);
            cache_mutex.lock();
            cache_host->put(site_url.host.value(), result::pages_limit);
            cache_mutex.unlock();
            thread->second = false;
            continue;
        }

        auto result = work(url_);

        if (result == result::null_or_limit) {
            cache_mutex.lock();
            cache_host->put(site_url.text, result::null_or_limit);
            cache_mutex.unlock();
        } else if (result == result::added || result == result::disallowed_robots) {
            //delay, after http request
            std::this_thread::sleep_for(std::chrono::seconds(config.crawler_.delay_time_s));
        }

        thread->second = false;
    }
}
worker::result worker::work(url &url_) {
    http::url site_url(url_.site_url);
    http::url owner_url(url_.owner_url);

    site_url.parse();
    owner_url.parse();

    if (config.crawler_.is_check_robots_txt) {
        auto robots_txt_body = get_added_robots_txt(site_url.host.value()).value_or("");
        bool is_checked = true;

        if (robots_txt_body.empty()) {
            robots_txt_body = get_robots_txt(site_url).value_or("");
            auto robots_txt_body_length = robots_txt_body.length();

            if (robots_txt_body_length > 0 && robots_txt_body_length < config.crawler_.max_robots_txt_symbols) {
                const auto json = robots_txt_json(robots_txt_body, site_url.host.value());
                if (!json) return result::null_or_limit;
                config.db_.robots.add(*json);
            } else {
                is_checked = false;
            }
        }

        if (is_checked && !is_allowed_in_robots(robots_txt_body, site_url)) {
            if_debug_print(logger::type::error, "disallowed robots.txt", site_url.text);
            return result::disallowed_robots;
        }
    }

    auto request_result = site(site_url);
    auto response = request_result.response;
    auto response_length = response->length();

    //if_debug_print(logger::type::info, "http://localhost:8080/response length = " + to_string(response_length), site_url.text);
    if_debug_print(logger::type::info,
                   "response code = "  + to_string(request_result.code) +
                   " | curl code = " + to_string(request_result.curl_code), site_url.text);

    if (request_result.location_url.has_value() && !request_result.location_url->empty()) {
        url_.site_url = *request_result.location_url;
        site_url.text = *request_result.location_url;
        site_url.parse();
    }

    if (std::find(ALLOWED_CODES.begin(), ALLOWED_CODES.end(), request_result.code) == ALLOWED_CODES.end()) {
        if_debug_print(logger::type::error, "code not allowed", site_url.text);
        return result::null_or_limit;
    }
    if (!response || response_length < 1 || response_length >= config.crawler_.max_page_symbols) {
        if_debug_print(logger::type::error, "response = null | length < 1 | >= limit", site_url.text);
        return result::null_or_limit;
    }

    auto document = parse_html(response.value());

    if (!document) {
        if_debug_print(logger::type::error, "parse_html null", site_url.text);
        return result::null_or_limit;
    }

    auto body = lxb_dom_interface_node(document.value()->body);

    if (body == nullptr) {
        if_debug_print(logger::type::error, "site has not body", site_url.text);
        return result::null_or_limit;
    }

    std::string title = lxb_string_to_std(lxb_html_document_title(document.value(), nullptr)).value_or("");
    //const std::string content = lxb_string_to_std(lxb_dom_node_text_content(body, nullptr)).value_or("");
    std::string desc = get_desc("name", "description", document.value()); //meta tag

    if (desc.empty()) {
        desc = get_desc("http-equiv", "description", document.value()); //meta tag
    }
    if (desc.empty()) {
        desc.append(compute_desc("h1", document.value())); //h1 tags
    }
    if (title.empty() && desc.empty()) {
        if_debug_print(logger::type::error, "title & desc are empty", site_url.text);
        return result::null_or_limit;
    }

    bool has_trackers = false;

    for (const auto &s : TRACKER_PATTERNS) {
        if (contains(response.value(), s)) {
            has_trackers = true;
            break;
        }
    }

    const auto json = website_json(title, site_url.text, site_url.host.value(), desc, has_trackers);

    if (!json) {
        if_debug_print(logger::type::error, "generate json (add website)", site_url.text);
        return result::null_or_limit;
    }

    config.db_.websites.add(json.value());

    //print added url
    std::cout << logger::yellow << "[" << site_url.text << "]" << std::endl;

    auto collection = lxb_dom_collection_make(&(document.value())->dom_document, 16);
    lxb_dom_elements_by_tag_name(lxb_dom_interface_element(body), collection, std_string_to_lxb("a"), 1);
    const auto a_length = collection->array.length;

    for (size_t i = 0; i < a_length; i++) {
        auto element = lxb_dom_collection_element(collection, i);
        const auto href_value = lxb_string_to_std(lxb_dom_element_get_attribute(element, std_string_to_lxb("href"), 4, nullptr));

        if (!href_value || href_value.value() == site_url.text || starts_with(href_value.value(), "#")) {
            continue;
        }

        http::url href_url(href_value.value());
        href_url.parse();

        url new_url;

        if (!starts_with(href_value.value(), "http")) {
            new_url.site_url = href_url.text;
            new_url.owner_url = site_url.text;
        } else {
            new_url.site_url = href_value.value();
        }

        queue_mutex.lock();
        queue->push(new_url);
        queue_mutex.unlock();
    }

    if (a_length > 0) lxb_dom_collection_destroy(collection, true);

    lxb_html_document_destroy(document.value());
    return result::added;
}