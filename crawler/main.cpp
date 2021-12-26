#include <librengine/config.h>
#include <librengine/crawler/worker.h>
#include <librengine/str.h>

using namespace librengine;

void easy_start(const config::crawler &config) {
    curl_global_init(CURL_GLOBAL_ALL);      //https://stackoverflow.com/questions/6087886
    using namespace librengine;
    int deep = 0;

    auto worker = std::make_shared<crawler::worker>(config, opensearch::client(config.opensearch_url));
    worker->main_thread(config.start_site_url, deep);

    curl_global_cleanup();                  //https://curl.se/libcurl/c/curl_global_cleanup.html
}

int main(int argc, char **argv) {
    if (argc <= 2) {
        std::cout << "Usage: bin [start_site] [config_path]\nExample: ./crawler https://www.gnu.org ../../config.json" << std::endl;
        return 1;
    }

    config::crawler config;
    config.start_site_url = argv[1];
    config.load_from_file(argv[2]);
    std::string line = std::string(25, '=');

    std::cout << line << "CFG" << line << std::endl
              << config.to_str()  << std::endl
              << line << "===" << line << std::endl;

    easy_start(config);

    return 0;
}