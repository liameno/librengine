#include <optional>
#include <librengine/structs.h>
#include <librengine/config.h>
#include <librengine/logger.h>
#include <librengine/json.hpp>
#include <librengine/str.h>
#include <librengine/str_impl.h>
#include <librengine/http.h>
#include <librengine/typesense.h>
#include <librengine/encryption.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <map>

#include "../third_party/httplib.h"

#ifndef PAGES_H
#define PAGES_H

namespace website {
    using namespace librengine;
    using namespace httplib;

    class pages {
    private:
        config::all config;
        encryption::rsa rsa;
        std::map<std::string, std::string> rsa_public_keys;
    public:
        pages(const config::all &config);
        void init();
    
        void set_variables(std::string &page_src);

        void update(const std::string &id, const std::string &field, const size_t &value);
        void update(const std::string &id, const std::string &field, const std::string &value);
        size_t get_number_field_value(const std::string &id, const std::string &field);
        std::optional<std::vector<search_result>> search(const std::string &q, const size_t &p);
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