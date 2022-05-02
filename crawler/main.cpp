#include <librengine/config.h>
#include <librengine/str.h>
#include <librengine/logger.h>

#include "include/worker.h"

using namespace librengine;

void easy_start(const config::crawler &config, const config::db &db) {
    curl_global_init(CURL_GLOBAL_ALL); //https://stackoverflow.com/questions/6087886

    int deep = 0;
    auto w = std::make_shared<worker>(config, db);
    w->main_thread(config.start_site_url, deep);

    curl_global_cleanup(); //https://curl.se/libcurl/c/curl_global_cleanup.html
}

int main(int argc, char **argv) {
    if (argc <= 2) {
        std::cout << "Usage: bin [start_site] [config_path]\nExample: ./crawler https://www.gnu.org ../../config.json" << std::endl;
        return 1;
    }

    config::crawler config;
    config::db db;

    config.load_from_file(argv[2]);
    db.load_from_file(argv[2]);

    config.start_site_url = argv[1];

    std::string line = std::string(25, '=');

    std::cout   << logger::white << line << logger::green << "CFG" << logger::white << line << std::endl
                << logger::reset << config.to_str()  << std::endl
                << logger::white << line << "===" << logger::white << line << std::endl;

    easy_start(config, db);

    return 0;
}