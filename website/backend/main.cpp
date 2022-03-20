#include "pages.h"

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

    std::cout << line << "CFG" << line << std::endl
    << config.to_str()  << std::endl
    << line << "===" << line << std::endl;

    auto client = librengine::opensearch::client("http://localhost:9200");

    Server *server = new Server();
    server->set_mount_point("/", "../../frontend/");

    server->Get("/home", [&](const httplib::Request &req, httplib::Response &res) { pages::home(req, res, config); });
    server->Get("/search", [&](const httplib::Request &req, httplib::Response &res) { pages::search(req, res, client, config); });
    server->Get("/node/info", [&](const httplib::Request &req, httplib::Response &res) { pages::node_info(req, res, client); });
    server->Get("/node/admin_panel", [&](const httplib::Request &req, httplib::Response &res) { pages::node_admin_panel(req, res, client); });
    server->Get("/api/plus_rating", [&](const httplib::Request &req, httplib::Response &res) { pages::api_plus_rating(req, res, client); });
    server->Get("/api/minus_rating", [&](const httplib::Request &req, httplib::Response &res) { pages::api_minus_rating(req, res, client); });
    server->Get("/api/search", [&](const httplib::Request &req, httplib::Response &res) { pages::api_search(req, res, client); });
    server->Get("/api/node/info", [&](const httplib::Request &req, httplib::Response &res) { pages::api_node_info(req, res, client); });
    server->Get(".*", pages::not_found);

    std::cout << "The server was started at http://127.0.0.1:" << config.port << std::endl;
    server->listen("0.0.0.0", config.port);

    return 0;
}