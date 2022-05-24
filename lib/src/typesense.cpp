#include "../include/typesense.h"

namespace librengine {
    typesense::typesense() = default;
    typesense::typesense(const std::string &url, const std::string &collection_name, const std::string &api_key) {
        this->url = url;
        this->collection_name = collection_name;
        this->api_key = api_key;
    }

    std::string typesense::add(const std::string &json) {
        std::string request_url = this->url + "/collections/" + this->collection_name + "/documents";
        http::request request(request_url, json, "POST", false);
        request.options.headers->emplace_back("Content-Type: application/json");
        request.options.headers->emplace_back("X-TYPESENSE-API-KEY", api_key);
        request.perform();

        return request.result.response.value_or("");
    }

    std::string typesense::update(const std::string &json) {
        std::string request_url = this->url + "/collections/" + this->collection_name + "/documents/?action=upsert";
        http::request request(request_url, json, "POST", false);
        request.options.headers->emplace_back("Content-Type: application/json");
        request.options.headers->emplace_back("X-TYPESENSE-API-KEY", api_key);
        request.perform();

        return request.result.response.value_or("");
    }

    std::string typesense::get(const int &id) {
        std::string request_url = this->url + "/collections/" + this->collection_name + "/documents/" + std::to_string(id);
        http::request request(request_url, "", "GET", false);
        request.options.headers->emplace_back("X-TYPESENSE-API-KEY", api_key);
        request.perform();

        return request.result.response.value_or("");
    }

    std::string typesense::search(const std::string &q, const std::string &query_by, const std::map<std::string, std::string> &options) {
        std::string request_url = this->url + "/collections/" + this->collection_name + "/documents/search/";
        request_url.append("?q=" + q);
        request_url.append("&query_by=" + query_by);

        for (const auto &option : options) {
            request_url.append("&" + option.first + "=" + option.second);
        }

        http::request request(request_url, "", "GET", false);
        request.options.headers->emplace_back("X-TYPESENSE-API-KEY", api_key);
        request.perform();

        return request.result.response.value_or("");
    }
}