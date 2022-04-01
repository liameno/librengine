#include <optional>
#include <librengine/config.h>
#include <librengine/opensearch.h>
#include <librengine/logger.h>
#include <librengine/json.hpp>
#include <librengine/str.h>
#include <librengine/str_impl.h>
#include <librengine/http.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <map>

#include "../third_party/httplib.h"
#include "encryption.h"

#ifndef PAGES_H
#define PAGES_H

namespace backend {
    using namespace librengine;
    using namespace httplib;

    class pages {
    public:
        struct search_result {
            std::string id;
            std::string title;
            std::string url;
            std::string desc;
            size_t rating;
            bool has_ads;
            bool has_analytics;
        };
    private:
        encryption::rsa rsa;
        config::website config;
        opensearch::client client;

        std::map<std::string, std::string> rsa_public_keys;
    public:
        pages(const config::website &config, opensearch::client &client);
        void init();
    
        void set_variables(std::string &page_src);

        void update(const std::string &id, const std::string &field, const size_t &value);
        void update(const std::string &id, const std::string &field, const std::string &value);
        size_t get_number_field_value(const std::string &id, const std::string &field);
        /*size_t get_last_added_website_date(opensearch::client &client);*/
        std::optional<std::vector<search_result>> search(const std::string &q, const size_t &s);
        size_t get_field_count(const std::string &field);

        void home(const Request &request, Response &response);
        void search(const Request &request, Response &response);
        void node_info(const Request &request, Response &response);
        void node_admin_panel(const Request &request, Response &response);
        void api_get_rsa_public_key(const Request &request, Response &response);
        void api_plus_rating(const Request &request, Response &response);
        void api_minus_rating(const Request &request, Response &response);
        void api_search(const Request &request, Response &response);
        void api_node_info(const Request &request, Response &response);
        void not_found(const Request &request, Response &response);
    };
}

#endif