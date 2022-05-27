#ifndef ROBOTS_TXT_H
#define ROBOTS_TXT_H

#include "http.h"

namespace librengine {
    class user_agent {
    public:
        std::string agent;
        std::vector<std::string> allow_list;
        std::vector<std::string> disallow_list;
        float crawl_delay;
    public:
        static bool match(const std::string &pattern, const std::string &expression);
    public:
        explicit user_agent(const std::string &agent);

        bool allowed(const std::string &path);
        bool allowed(const http::url &url);
    };

    class robots_txt {
    private:
        std::string text;
    public:
        std::vector<user_agent> agents;
    public:
        explicit robots_txt(const std::string &text);
        void parse();

        bool allowed(const std::string &path, const std::string &agent);
        bool allowed(const http::url &url, const std::string &user_agent);
    };
}

#endif