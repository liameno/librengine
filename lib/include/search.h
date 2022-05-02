#ifndef SEARCH_H
#define SEARCH_H

#include <cstring>
#include <thread>

#include "structs.h"
#include "logger.h"
#include "json.hpp"
#include "str.h"
#include "str_impl.h"
#include "http.h"
#include "config.h"

namespace librengine {
    class search {
    private:
        config::all config;
    public:
        std::map<std::string, std::string> rsa_public_keys;
    public:
        search(const config::all &config);

        std::vector<search_result> local(const std::string &q, const size_t &p);
        std::vector<search_result> nodes(const std::string &q, const size_t &p);
    };
}

#endif