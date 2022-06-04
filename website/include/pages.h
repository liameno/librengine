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
#include <librengine/search.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <memory>
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
        std::shared_ptr<search> search_;

        std::string center_result_src_format;
    public:
        explicit pages(const config::all &config);
        ~pages();

        void init();
    
        void set_variables(std::string &page_src);

        void update(const std::string &id, const std::string &field, const size_t &value);
        void update(const std::string &id, const std::string &field, const std::string &value);
        size_t get_number_field_value(const std::string &id, const std::string &field);
        size_t get_field_count(const std::string &field);

        void home_p(const Request &request, Response &response);
        void search_p(const Request &request, Response &response);
        void node_info_p(const Request &request, Response &response);
        void api_get_rsa_public_key(const Request &request, Response &response);
        void api_plus_rating(const Request &request, Response &response);
        void api_minus_rating(const Request &request, Response &response);
        void api_search(const Request &request, Response &response);
        void api_node_info(const Request &request, Response &response);
        void not_found(const Request &request, Response &response);
    };
}

#endif