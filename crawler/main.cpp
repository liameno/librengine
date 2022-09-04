#include "../../lib/include/config.h"

#include "include/worker.h"

int main(int argc, char **argv) {
    using namespace librengine;

    if (argc <= 3) {
        std::cout << "Usage: bin [sites] [threads_count] [config]\nExample: ./crawler \"$(cat sites.txt)\" 5 \"$(cat config.json)\"" << std::endl;
        return 1;
    }

    config::all config;
    config.load_from_content(argv[3]);

    //https://stackoverflow.com/questions/6087886
    curl_global_init(CURL_GLOBAL_ALL);

    auto splited = split(argv[1], "\n");

    auto threads_count = std::stoi(argv[2]);

    worker w(config);

    for (auto s : splited) {
        if (!starts_with(s, "http")) {
            s.insert(0, "http://");
        }

        worker::url new_url;
        new_url.site_url = s;
        w.queue->push(new_url);
    }

    w.start_threads(threads_count);

    //https://curl.se/libcurl/c/curl_global_cleanup.html
    curl_global_cleanup();

    return 0;
}