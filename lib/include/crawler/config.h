#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

#include "../http.h"
#include "../str_impl.h"

namespace librengine::crawler {
    struct config {
        std::string user_agent                = "librengine";
        std::string start_site_url            = "https://www.gnu.org";
        std::string opensearch_url            = "http://localhost:9200";      //without '/'

        std::optional<http::proxy> proxy      = std::nullopt; //http::proxy{"127.0.0.1:9050", http::proxy_type::socks5} or socks5://127.0.0.1:9050

        size_t recursive_deep_max             = 3;
        size_t load_page_timeout_s            = 10;
        size_t update_time_site_info_s_after  = 864000;   //10 days
        size_t delay_time_s                   = 3;
        size_t limit_pages_site               = 300;

        size_t limit_page_symbols             = 50000000;  //50 mb
        size_t limit_robots_txt_symbols       = 3000;
        //size_t limit_sitemap_symbols        = 10000;

        size_t is_http_to_https               = true;     //https://en.wikipedia.org/wiki/HTTPS
        size_t is_check_robots_txt            = true;     //https://en.wikipedia.org/wiki/Robots_exclusion_standard
        //size_t is_check_sitemap             = true;     //https://en.wikipedia.org/wiki/Sitemaps

        std::string to_str() const {
            const std::string format = "UA={0}\nStartSiteUrl={1}\nOSUrl={2}\nProxy={3}\nRDeepMax={4}"
                                       "\nLPageTimeoutS={5}\nUpdateTimeSISAfter={6}\nDelayTimeS={7}\nLimitPagesS={8}\nLimitPageSym={9}"
                                       "\nLimitRobotsTSym={10}\nIsHttpToHttps={11}\nIsCheckRobots={12}";
            return str::format(format, user_agent, start_site_url, opensearch_url, (proxy) ? proxy->compute_curl_format() : "null", recursive_deep_max,
            load_page_timeout_s, update_time_site_info_s_after, delay_time_s, limit_pages_site, limit_page_symbols, limit_robots_txt_symbols,
            is_check_robots_txt, is_check_robots_txt);
        }
    };
}

#endif