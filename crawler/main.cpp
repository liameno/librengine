#include <librengine/config.h>

#include "include/worker.h"

int main(int argc, char **argv) {
    using namespace librengine;

    if (argc <= 2) {
        std::cout << "Usage: bin [sites_path] [config_path]\nExample: ./crawler ../../sites.txt ../../config.json" << std::endl;
        return 1;
    }

    config::all config;
    config.load_from_file(argv[2]);

    //https://stackoverflow.com/questions/6087886
    curl_global_init(CURL_GLOBAL_ALL);

    auto content = config::helper::get_file_content(argv[1]);
    auto splited = split(content, "\n");
    auto w = std::make_shared<worker>(config);

    for (const auto &s : splited) {
        w->queue.push({s});
    }

    w->main_thread();

    //https://curl.se/libcurl/c/curl_global_cleanup.html
    curl_global_cleanup();

    return 0;
}