#include "robots_txt.h"

#include "str.h"

#include <algorithm>

namespace librengine {
    bool user_agent::match(const std::string &pattern, const std::string &expression) {
        auto pattern_size = pattern.length();
        auto expression_size = expression.length();

        std::vector<size_t> vector_pos(expression_size + 1);
        size_t pos = 1;

        for (int i = 0; i < pattern_size; ++i) {
            char c = pattern[i];

            if (c == '$' && i + 1 == pattern_size) {
                return vector_pos[pos - 1] == expression_size;
            }
            if (c == '*') {
                pos = expression_size - vector_pos[0] + 1;

                for (int j = 1; j < pos; j++) {
                    vector_pos[j] = vector_pos[j - 1] + 1;
                }
            } else {
                int tmp_pos = 0;

                for (int j = 0; j < pos; j++) {
                    auto c_pos = vector_pos[j];

                    if (c_pos < expression_size && expression[c_pos] == c) {
                        vector_pos[tmp_pos] = c_pos + 1;
                        ++tmp_pos;
                    }
                }

                if (tmp_pos == 0) return false;
                pos = tmp_pos;
            }
        }

        return true;
    }

    user_agent::user_agent(const std::string &agent) {
        this->agent = agent;
        crawl_delay = 0;
    }

    bool user_agent::allowed(const std::string &path) {
        for (const auto &allow : allow_list) {
            if (match(allow, path)) {
                return true;
            }
        }

        for (const auto &disallow : disallow_list) {
            if (match(disallow, path)) {
                return false;
            }
        }

        return true;
    }
    bool user_agent::allowed(const http::url &url) {
        if (!url.path) return false;
        std::string path = *url.path;
        return allowed(path);
    }

    robots_txt::robots_txt(const std::string &text) {
        this->text = text;
        agents.emplace_back("");
    }
    void robots_txt::parse() {
        auto splited = split(text, "\n");

        for (const auto &pair : splited) {
            auto splited_pair = split(pair, ":");
            auto splited_pair_size = splited_pair.size();

            if (splited_pair_size != 2) continue;

            auto key = to_lower_copy(splited_pair[0]);
            auto value = to_lower_copy(splited_pair[1]);

            trim(key);
            trim_start(value);

            if (!value.empty()) trim_end(value);
            auto comment_index = value.find('#');

            if (comment_index != -1) {
                value = value.substr(0, comment_index);
                trim_end(value);
            }

            if (key.empty()) continue;
            if (key != "disallow" && value.empty()) continue;

            auto &current_agent = agents.back();

            if (key == "user-agent") {
                agents.emplace_back(value);
            }
            else if (key == "allow") {
                current_agent.allow_list.push_back(value);
            }
            else if (key == "disallow") {
                if (value.empty()) current_agent.allow_list.emplace_back("/");
                else current_agent.disallow_list.push_back(value);
            }
            else if (key == "crawl-delay") {
                try {
                    current_agent.crawl_delay = std::stof(value);
                } catch (const std::exception &e) {
                    //current_agent.crawl_delay = 0; (def)
                }
            }
        }
    }

    bool robots_txt::allowed(const std::string &path, const std::string &agent) {
        auto found = std::find_if(agents.begin(), agents.end(), [&](user_agent &u){ return u.agent == agent; });
        if (found == agents.end()) found = std::find_if(agents.begin(), agents.end(), [&](user_agent &u){ return u.agent == "*"; });
        if (found == agents.end()) found = std::find_if(agents.begin(), agents.end(), [&](user_agent &u){ return u.agent == ""; });

        return found->allowed(path);
    }
    bool robots_txt::allowed(const http::url &url, const std::string &user_agent) {
        if (!url.path) return false;
        std::string path = *url.path;

        return allowed(path, user_agent);
    }
}