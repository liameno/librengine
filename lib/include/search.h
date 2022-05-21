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
#include "encryption.h"

namespace librengine {
    class search {
    private:
        config::all config;
        std::map<std::string, std::string> rsa_public_keys;
    public:
        encryption::rsa rsa;
        std::string rsa_public_key;
        std::string rsa_public_key_base64;
    public:
        explicit search(const config::all &config);
        void init();

        void remove_html_tags(std::string &html);

        std::vector<search_result> local(const std::string &q, const size_t &p);
        std::vector<search_result> nodes(std::string &query, const size_t &page, const bool &is_encryption_enabled, const std::string &encryption_key);
    };
}

#endif