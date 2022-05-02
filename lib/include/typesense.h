#ifndef TYPESENSE_H
#define TYPESENSE_H

#include <string>
#include <memory>
#include <map>

#include "http.h"

namespace librengine {
    class typesense {
    private:
        std::string url;
        std::string collection_name;
        std::string api_key;
    public:
        typesense();
        typesense(const std::string &url, const std::string &collection_name, const std::string &api_key);

        std::string add(const std::string &json);
        std::string update(const std::string &json);

        std::string get(const int &id);
        std::string search(const std::string &q, const std::string &query_by, const std::map<std::string, std::string> &options = {});
    };
}

#endif