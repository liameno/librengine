#include "include/pages.h"

int main(int argc, char **argv) {
    using namespace librengine;
    using namespace backend;
    using namespace httplib;

    if (argc <= 1) {
        std::cout << "Usage: bin [config_path]\nExample: ./backend config.json" << std::endl;
        return 1;
    }

    config::website config;
    config.load_from_file(argv[1]);
    std::string line = std::string(25, '=');

    std::cout   << logger::white << line << logger::green << "CFG" << logger::white << line << std::endl
                << logger::reset << config.to_str()  << std::endl
                << logger::white << line << "===" << logger::white << line << std::endl;

    auto client = librengine::opensearch::client("http://localhost:9200");

    auto server = std::make_shared<Server>();
    auto pages = std::make_shared<backend::pages>(config, client);

    std::thread server_thread([&] {
        server->set_mount_point("/", "../../frontend/");
        server->Get("/home", [&](const Request &req, Response &res) { pages->home(req, res); });
        server->Get("/search", [&](const Request &req, Response &res) { pages->search(req, res); });
        server->Get("/node/info", [&](const Request &req, Response &res) { pages->node_info(req, res); });
        server->Get("/node/admin_panel", [&](const Request &req, Response &res) { pages->node_admin_panel(req, res); });
        server->Get("/api/get_rsa_public_key", [&](const Request &req, Response &res) { pages->api_get_rsa_public_key(req, res); });
        server->Get("/api/plus_rating", [&](const Request &req, Response &res) { pages->api_plus_rating(req, res); });
        server->Get("/api/minus_rating", [&](const Request &req, Response &res) { pages->api_minus_rating(req, res); });
        server->Get("/api/search", [&](const Request &req, Response &res) { pages->api_search(req, res); });
        server->Get("/api/node/info", [&](const Request &req, Response &res) { pages->api_node_info(req, res); });
        server->Get(".*", [&](const Request &req, Response &res) { pages->not_found(req, res); });
        server->listen("0.0.0.0", config.port);
    });

    while(!server->is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    pages->init();

    std::cout << "The server was started at http://127.0.0.1:" << config.port << std::endl;
    server_thread.join();

    return 0;
}