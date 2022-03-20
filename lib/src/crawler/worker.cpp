#include "crawler/worker.h"

#include <lexbor/html/html.h>
#include <optional>
#include <thread>
#include <utility>

#include "crawler/helper.h"
#include "config.h"
#include "http.h"
#include "str.h"
#include "opensearch.h"
#include "../../third_party/rep-cpp/robots.h"

#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#define DEBUG true //TODO: false

enum class debug_type {
    info,
    error,
};

void if_debug_print(const debug_type &type, const std::string &text, const std::string &ident) {
    std::string type_s;
    std::string type_color;

    switch (type) {
        case debug_type::info:
            type_s = "INFO";
            type_color = CYAN;
            break;
        case debug_type::error:
            type_s = "ERROR";
            type_color = RED;
            break;
    }
#if DEBUG
std::cout << type_color << "[" << type_s << "] " << GREEN << text << WHITE << " [" << ident << "]" << std::endl;
#endif
}

namespace librengine::crawler {
    using namespace opensearch;

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

        if (this->current_config.is_http_to_https) {
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

    worker::worker(config::crawler config, opensearch::client opensearch_client) : current_config(std::move(config)), opensearch_client(std::move(opensearch_client)) {
        this->is_work = true;
    }

    worker::result worker::main_thread(const std::string &site_url, int &deep, const std::optional<http::url> &owner_url) {
        if (!is_work) {
            return result::work_false;
        }

        http::url url(str::to_lower(site_url));
        url.parse();

        std::string owner_url_text = (owner_url) ? owner_url->text : "";

        if (!normalize_url(url, owner_url_text)) {
            if_debug_print(debug_type::error, "normalize url", url.text);
            return result::error;
        }
        if (!url.host) {
            if_debug_print(debug_type::error, "host == null", url.text);
            return result::null_or_limit;
        }
        if (url.text == owner_url_text) {
            if_debug_print(debug_type::error, "url == owner", url.text);
            return result::already_added;
        }
        if (current_config.is_one_site && owner_url && url.host != owner_url->host) {
            return result::already_added;
        }
        if (helper::hints_count_added("url", url.text, current_config, opensearch_client) > 0) {
            if_debug_print(debug_type::error, "already added", url.text);
            return result::already_added;
        }

        size_t pages_count = helper::hints_count_added("host", *url.host, current_config, opensearch_client);

        if (pages_count >= this->current_config.max_pages_site) {
            if_debug_print(debug_type::error, "pages count >= limit", url.text);
            return result::pages_limit;
        }

        if (this->current_config.is_check_robots_txt) {
            auto robots_txt_body = helper::get_added_robots_txt(*url.host, current_config, opensearch_client).value_or("");
            bool is_checked = true;

            if (robots_txt_body.empty()) {
                robots_txt_body = helper::get_robots_txt(url, current_config).value_or("");
                auto robots_txt_body_length = robots_txt_body.length();

                if (robots_txt_body_length > 1 && robots_txt_body_length < this->current_config.max_robots_txt_symbols) {
                    const auto json = helper::compute_robots_txt_json(robots_txt_body, *url.host);

                    if (!json) {
                        return result::null_or_limit;
                    }

                    const auto path = opensearch::client::path_options("robots_txt/_doc");
                    const auto type = opensearch::client::request_type::POST;

                    //add a robots_txt to the opensearch
                    opensearch_client.custom_request(path, type, json);
                } else {
                    is_checked = false;
                }
            }

            if (is_checked && !helper::is_allowed_in_robots(robots_txt_body, url.text, current_config)) {
                return result::disallowed_robots;
            }
        }

        auto request_result = helper::site(url, current_config);
        auto response = request_result.response;
        auto response_length = response->length();

        if_debug_print(debug_type::info, "response length = " + str::to_string(response_length), url.text);
        if_debug_print(debug_type::info, "response code = " + str::to_string(request_result.code), url.text);
        if_debug_print(debug_type::info, "curl code = " + str::to_string(request_result.curl_code), url.text);

        if (request_result.code != 200) {
            if_debug_print(debug_type::error, "code != 200", url.text);
            return result::null_or_limit;
        }
        if (!response || response_length < 1 || response_length >= this->current_config.max_page_symbols) {
            if_debug_print(debug_type::error, "response = null || length < 1 || >= limit", url.text);
            return result::null_or_limit;
        }

        auto document = helper::parse_html(*response);

        if (!document) {
            return result::null_or_limit;
        }

        auto body = lxb_dom_interface_node((*document)->body);

        if (body == nullptr) {
            return result::null_or_limit;
        }

        const std::string title = helper::lxb_string_to_std(lxb_html_document_title((*document), nullptr)).value_or("");
        //const std::string content = lxb_string_to_std(lxb_dom_node_text_content(body, nullptr)).value_or("");
        std::string desc = helper::get_desc(*document); //by meta tag

        bool has_ads = false;
        bool has_analytics = false;

        const std::vector<std::string> detect_ads_strings = {
                "g.doubleclick.net ", "adservice.google.com",
                "amazon-adsystem.com/aax2/apstag.js", "yandex.ru/ads/system/context.js",
                "\"adServer\":\"", "googletag.pubads", "adsbygoogle.js",
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

        if (desc.empty()) {
            //compute desc from h1 tags
            desc.append(helper::compute_desc("h1", *document));
        }
        if (title.empty() && desc.empty()) {
            if_debug_print(debug_type::error, "title & desc are empty", url.text);
            return result::null_or_limit;
        }

        const auto json = helper::compute_website_json(title, url.text, *url.host, desc, has_ads, has_analytics);

        if (!json) {
            return result::null_or_limit;
        }

        const auto path = opensearch::client::path_options("website/_doc");
        const auto type = opensearch::client::request_type::POST;

        //add a website to the opensearch
        opensearch_client.custom_request(path, type, json);

        //print added url
        std::cout << YELLOW << "[" << url.text << "]" << std::endl;

        if (deep < this->current_config.max_recursive_deep) {
            auto collection = lxb_dom_collection_make(&(*document)->dom_document, 16);
            lxb_dom_elements_by_tag_name(lxb_dom_interface_element(body), collection, helper::std_string_to_lxb("a"), 1);
            const auto a_length = collection->array.length;
            std::vector<std::string> pages_limit_hosts;
            ++deep;

            for (size_t i = 0; i < a_length; i++) {
                auto element = lxb_dom_collection_element(collection, i);
                const auto href_value = helper::lxb_string_to_std(lxb_dom_element_get_attribute(element, helper::std_string_to_lxb("href"), 4, nullptr));

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
                    if (current_config.is_one_site && href_url.host != url.host) {
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
                    std::this_thread::sleep_for(std::chrono::seconds(this->current_config.delay_time_s));
                }
            }

            --deep;

            if (a_length > 0) {
                lxb_dom_collection_destroy(collection, true);
            }
        }

        lxb_html_document_destroy(*document);
        return result::added;
    }
}
