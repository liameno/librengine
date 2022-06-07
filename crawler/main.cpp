#include <librengine/config.h>

#include "include/worker.h"

int main(int argc, char **argv) {
    using namespace librengine;

    if (argc <= 3) {
        std::cout << "Usage: bin [sites_path] [threads_count] [config_path]\nExample: ./crawler ../../sites.txt 5 ../../config.json" << std::endl;
        return 1;
    }

    config::all config;
    config.load_from_file(argv[3]);

    //https://stackoverflow.com/questions/6087886
    curl_global_init(CURL_GLOBAL_ALL);

    auto content = config::helper::get_file_content(argv[1]);
    auto splited = split(content, "\n");

    auto threads_count = std::stoi(argv[2]);

    worker w(config);

    for (const auto &s : splited) {
        worker::url new_url;
        new_url.site_url = s;
        w.queue->push(new_url);
    }

    w.start_threads(threads_count);

    //https://curl.se/libcurl/c/curl_global_cleanup.html
    curl_global_cleanup();

    return 0;
}