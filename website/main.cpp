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

    nlohmann::json a;
    a["results"][0]["title"] = std::string("捐款 | LibreOffice 正體中文站 - 自由的辦�...");
    a["results"][0]["url"] = "https://zh-tw.libreoffice.org/donate/";
    a["results"][0]["desc"] = "Donate, donation, donations, funding, help, support, LibreOffice";
    a["results"][0]["rating"] = "100";
    a["results"][0]["id"] = "9";
    a["results"][0]["has_trackers"] = "0";
    a["results"][1]["title"] = std::string("Qu'est-ce que LibreOffice | Communauté LibreOffice ...");
    a["results"][1]["url"] = "https://fr.libreoffice.org/discover/libreoffice/";
    a["results"][1]["desc"] = "LibreOffice, Free Office Suite, Fun Project, Fantastic People, Writer, Calc, Impress, Draw, Base, Charts, Diagrams, extensions, templates, word processor, text editor, spreadsheet, presentation, database, documents, Document Foundation";
    a["results"][1]["rating"] = "100";
    a["results"][1]["id"] = "6";
    a["results"][1]["has_trackers"] = "0";
    std::cout << a.dump();
    //return 0;

    std::thread server_thread([&] {
        server->set_mount_point("/", "../frontend/");
        server->Get("/home", [&](const Request &req, Response &res) { pages->home_p(req, res); });
        server->Get("/search", [&](const Request &req, Response &res) { pages->search_p(req, res); });
        server->Get("/node/info", [&](const Request &req, Response &res) { pages->node_info_p(req, res); });
        server->Get("/api/get_rsa_public_key", [&](const Request &req, Response &res) { pages->api_get_rsa_public_key(req, res); });
        server->Get("/api/plus_rating", [&](const Request &req, Response &res) { pages->api_plus_rating(req, res); });
        server->Get("/api/minus_rating", [&](const Request &req, Response &res) { pages->api_minus_rating(req, res); });
        server->Get("/api/search", [&](const Request &req, Response &res) { pages->api_search(req, res); });
        server->Get("/api/node/info", [&](const Request &req, Response &res) { pages->api_node_info(req, res); });
        server->Get(".*", [&](const Request &req, Response &res) { pages->not_found(req, res); });
        server->listen("0.0.0.0", config.website_.port);
    });

    while(!server->is_running()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    pages->init();

    std::cout << "http://localhost:" << config.website_.port << "/" << std::endl;
    server_thread.join();

    return 0;
}