#include <librengine/config.h>
#include <librengine/str.h>
#include <librengine/logger.h>

#include "include/worker.h"

using namespace librengine;

void easy_start(const std::string &start_site_url, const config::all &config) {
    curl_global_init(CURL_GLOBAL_ALL); //https://stackoverflow.com/questions/6087886

    int deep = 0;
    auto w = std::make_shared<worker>(config);
    w->main_thread(start_site_url, deep);

    curl_global_cleanup(); //https://curl.se/libcurl/c/curl_global_cleanup.html
}

int main(int argc, char **argv) {
    if (argc <= 2) {
        std::cout << "Usage: bin [start_site] [config_path]\nExample: ./crawler https://www.gnu.org ../../config.json" << std::endl;
        return 1;
    }

    config::all config;
    config.load_from_file(argv[2]);

    easy_start(argv[1], config);

    return 0;
}