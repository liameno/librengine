#include "opensearch.h"

#include <utility>

namespace librengine::opensearch {
    void client::path_options::compute_full() {
        this->full.append(this->index);
        this->full.push_back('/');
        this->full.append(this->type);
        this->full.push_back('/');
        this->full.append(this->document);
    }

    client::path_options::path_options(const std::string &full) {
        this->full = full;
    }

    client::client(std::string url) : url(std::move(url)) {

    }

    std::string client::compute_url(const std::string &path) {
        std::string result = this->url;

        result.push_back('/');
        result.append(path);

        return result;
    }

    std::optional<std::string> client::custom_request(const client::path_options &path_options, const request_type &request_type, const std::optional<std::string> &data) {
        std::string type;

        if (request_type == request_type::GET) type = "GET";
        else if (request_type == request_type::POST) type = "POST";
        else if (request_type == request_type::PUT) type = "PUT";
        else if (request_type == request_type::DELETE) type = "DELETE";

        http::request request(compute_url(path_options.full), data.value_or(""), type, false);
        request.options.headers->emplace_back("Content-Type: application/json");
        request.perform();

        return request.result.response;
    }
}