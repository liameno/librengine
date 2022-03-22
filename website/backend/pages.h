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

#include "third_party/httplib.h"

#ifndef PAGES_H
#define PAGES_H

namespace backend::pages {
    using namespace librengine;

    struct search_result {
        std::string id;
        std::string title;
        std::string url;
        std::string desc;
        size_t rating;
        bool has_ads;
        bool has_analytics;
    };

    void set_variables(std::string &page_src);

    void update(const std::string &id, const std::string &field, const size_t &value, opensearch::client &client);
    void update(const std::string &id, const std::string &field, const std::string &value, opensearch::client &client);
    size_t get_number_field_value(const std::string &id, const std::string &field, opensearch::client &client);
    /*size_t get_last_added_website_date(opensearch::client &client);*/
    std::optional<std::vector<search_result>> search(const std::string &q, const size_t &s, opensearch::client &client);
    size_t get_field_count(const std::string &field, opensearch::client &client);

    void home(const httplib::Request &request, httplib::Response &response, const config::website &config);
    void search(const httplib::Request &request, httplib::Response &response, opensearch::client &client, const config::website &config);
    void node_info(const httplib::Request &request, httplib::Response &response, opensearch::client &client);
    void node_admin_panel(const httplib::Request &request, httplib::Response &response, opensearch::client &client);
    void api_plus_rating(const httplib::Request &request, httplib::Response &response, opensearch::client &client);
    void api_minus_rating(const httplib::Request &request, httplib::Response &response, opensearch::client &client);
    void api_search(const httplib::Request &request, httplib::Response &response, opensearch::client &client);
    void api_node_info(const httplib::Request &request, httplib::Response &response, opensearch::client &client);
    void not_found(const httplib::Request &request, httplib::Response &response);
}

#endif