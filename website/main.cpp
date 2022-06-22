#include "include/pages.h"

int main(int argc, char **argv) {
    using namespace librengine;
    using namespace httplib;

    if (argc <= 1) {
        std::cout << "Usage: bin [config_path]\nExample: ./backend config.json" << std::endl;
        return 1;
    }

    config::all config;
    config.load_from_file(argv[1]);

    auto server = std::make_shared<Server>();
    auto pages = std::make_shared<website::pages>(config);

    std::thread server_thread([&] {
        server->set_mount_point("/", "../frontend/");
        server->Get("/home", [&](lambda_args) { pages->home_p(req, res); });
        server->Get("/search", [&](lambda_args) { pages->search_p(req, res); });
        server->Get("/node/info", [&](lambda_args) { pages->node_info_p(req, res); });
        server->Get("/api/get_rsa_public_key", [&](lambda_args) { pages->api_get_rsa_public_key(req, res); });
        server->Get("/api/plus_rating", [&](lambda_args) { pages->api_plus_rating(req, res); });
        server->Get("/api/minus_rating", [&](lambda_args) { pages->api_minus_rating(req, res); });
        server->Get("/api/search", [&](lambda_args) { pages->api_search(req, res); });
        server->Get("/api/node/info", [&](lambda_args) { pages->api_node_info(req, res); });
        server->Get("/api/node/info/chart", [&](lambda_args) { pages->api_node_info_chart(req, res); });
        server->Get(".*", [&](lambda_args) { pages->not_found(req, res); });
        server->listen("0.0.0.0", (int)config.website_.port);
    });

    while(!server->is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    pages->init();

    std::cout << "http://localhost:" << config.website_.port << "/" << std::endl;
    server_thread.join();

    return 0;
}