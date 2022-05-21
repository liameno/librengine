#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>

namespace librengine {
    struct search_result {
        std::string id;
        std::string title;
        std::string url;
        std::string desc;
        size_t rating;
        bool has_ads;
        bool has_analytics;
        std::string node_url;
    };
}

#endif