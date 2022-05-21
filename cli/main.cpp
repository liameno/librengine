#include <librengine/config.h>
#include <librengine/logger.h>
#include <librengine/search.h>
#include <librengine/encryption.h>

int main(int argc, char **argv) {
    using namespace librengine;

    if (argc <= 2) {
        std::cout << "Usage: bin [query] [page] [config_path]\nExample: ./cli \"gnu\" 1 ../../config.json" << std::endl;
        return 1;
    }

    config::all config;
    config.load_from_file(argv[3]);

    std::string query = argv[1];
    size_t page = std::stoi(argv[2]);

    search search_(config);
    search_.init();

    auto results = search_.nodes(query, page, true);
    auto size = results.size();

    for (int i = 0; i < size; ++i) {
        const auto &result = results[i];

        const auto &id = result.id;
        const auto &title = result.title;
        const auto &url = result.url;
        const auto &desc = result.desc;
        const auto &rating = result.rating;
        const auto &has_ads = result.has_ads ? "has" : "hasn't";
        const auto &has_analytics = result.has_analytics ? "has" : "hasn't";
        const auto &node_url = result.node_url;

        std::cout << title << std::endl;
        std::cout << url << std::endl;
        std::cout << desc << std::endl;
        std::cout << rating << " rating" << std::endl;
        std::cout << node_url << " node url " << std::endl;
        std::cout << has_ads << " ads" << std::endl;
        std::cout << has_analytics << " analytics" << std::endl;
        std::cout << std::string(25, '=') << std::endl;
    }

    return 0;
}