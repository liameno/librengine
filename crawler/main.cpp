#include <librengine/crawler/config.h>
#include <librengine/crawler/worker.h>
#include <librengine/crawler/robots_txt.h>
#include <librengine/json.hpp>
#include <librengine/str.h>

using namespace librengine;

void easy_start(const crawler::config &config) {
    curl_global_init(CURL_GLOBAL_ALL);      //https://stackoverflow.com/questions/6087886
    using namespace librengine;
    int deep = 0;

    auto worker = std::make_shared<crawler::worker>(config, opensearch::client(config.opensearch_url));
    worker->main_thread(config.start_site_url, deep);

    curl_global_cleanup();                  //https://curl.se/libcurl/c/curl_global_cleanup.html
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        std::cout << "Usage: bin [start_site] [proxy = null] [recursive_deep_max = 3] [load_page_timeout_s = 10] [delay_time_s = 5] [limit_pages_site = 300]\nExample: ./crawler https://www.gnu.org socks5://127.0.0.1:9050" << std::endl;
        return 1;
    }

    crawler::config config;

    config.start_site_url                   = (argc > 1) ? argv[1] : "https://www.gnu.org";
    if (argc > 2) config.proxy              = http::proxy{argv[2]};

    config.recursive_deep_max               = (argc > 3) ? std::stoi(argv[3]) : 3;
    config.load_page_timeout_s              = (argc > 4) ? std::stoi(argv[4]) : 10;
    config.delay_time_s                     = (argc > 5) ? std::stoi(argv[5]) : 5;
    config.limit_pages_site                 = (argc > 6) ? std::stoi(argv[6]) : 300;

    config.user_agent                       = "librengine";
    config.opensearch_url                   = "http://localhost:9200";      //without '/'
    config.update_time_site_info_s_after    = 864000;   //10 days
    config.limit_page_symbols               = 50000000; //50 mb
    config.limit_robots_txt_symbols         = 3000;
    config.is_http_to_https                 = true;     //https://en.wikipedia.org/wiki/HTTPS
    config.is_check_robots_txt              = true;     //https://en.wikipedia.org/wiki/Robots_exclusion_standard

    std::cout   << std::string(25, '=')     << "CFG"    << std::string(25, '=') << std::endl
                << config.to_str()          << std::endl
                << std::string(25, '=')     << "==="    << std::string(25, '=') << std::endl;

    easy_start(config);

    return 0;
}