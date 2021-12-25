#include "pages.h"

int main(int argc, char **argv) {
    using namespace backend;
    using namespace httplib;

    if (argc <= 1) {
        std::cout << "Usage: bin [port]\nExample: ./backend 8080" << std::endl;
        return 1;
    }

    int port;

    try {
        port = std::stoi(argv[1]);

        if (port <= 0) {
            std::cout << "Port == 0" << std::endl;
            return 3;
        }
    } catch (const std::exception &e) {
        std::cout << "Port parse error" << std::endl;
        return 2;
    }

    auto client = librengine::opensearch::client("http://localhost:9200");

    Server server;
    server.set_mount_point("/", "../../frontend/");

    server.Get("/home", pages::home);
    server.Get("/search", [&client](const httplib::Request &req, httplib::Response &res) { pages::search(req, res, client); });
    server.Get("/api/plus_rating", [&client](const httplib::Request &req, httplib::Response &res) { pages::api_plus_rating(req, res, client); });
    server.Get("/api/minus_rating", [&client](const httplib::Request &req, httplib::Response &res) { pages::api_minus_rating(req, res, client); });
    server.Get("/node/info", [&client](const httplib::Request &req, httplib::Response &res) { pages::node_info(req, res, client); });
    server.Get("/api/search", [&client](const httplib::Request &req, httplib::Response &res) { pages::api_search(req, res, client); });
    server.Get("/api/node/info", [&client](const httplib::Request &req, httplib::Response &res) { pages::api_node_info(req, res, client); });
    server.Get(".*", pages::not_found);

    std::cout << "The server was started at http://127.0.0.1:" << port << std::endl;
    server.listen("0.0.0.0", port);

    return 0;
}