#include <librengine/config.h>

#include "include/worker.h"

using namespace librengine;

int main(int argc, char **argv) {
    using namespace librengine;

    if (argc <= 2) {
        std::cout << "Usage: bin [start_site] [config_path]\nExample: ./crawler https://www.gnu.org ../../config.json" << std::endl;
        return 1;
    }

    config::all config;
    config.load_from_file(argv[2]);

    //https://stackoverflow.com/questions/6087886
    curl_global_init(CURL_GLOBAL_ALL);

    auto w = std::make_shared<worker>(config);
    w->queue.push({argv[1], ""});
    w->main_thread();

    //https://curl.se/libcurl/c/curl_global_cleanup.html
    curl_global_cleanup();

    return 0;
}