#ifndef OPENSEARCH_H
#define OPENSEARCH_H

#include <string>
#include <memory>

#include "http.h"

namespace librengine::opensearch {
    class client {
    public:
        enum class request_type {
            GET,
            POST,
            PUT,
            DELETE,
        };

        struct path_options {
            std::string full;
            std::string index;
            std::string type;
            std::string document;

            void compute_full();
            explicit path_options(const std::string &full);
        };
    private:
        std::string url;
    private:
        std::string compute_url(const std::string &path);
    public:
        explicit client(std::string url = "http://localhost:9200");
        std::optional<std::string> custom_request(const path_options &path_options, const request_type &request_type,const std::optional<std::string> &data = std::nullopt);
    };
}

#endif