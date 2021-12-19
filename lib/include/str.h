#ifndef STR_H
#define STR_H

#include <algorithm>
#include <string>
#include <vector>

namespace librengine::str {
    std::string to_string(const std::string &value);
    char get_first_char(const std::string &s);
    char get_last_char(const std::string &s);
    void remove_first_char(std::string &s);
    void remove_last_char(std::string &s);

    std::string trim_start(const std::string &s);
    std::string trim_end(const std::string &s);
    std::string trim(const std::string &s);
    std::vector<std::string> split(const std::string &s, const std::string &delimiter);
    std::string to_lower(const std::string &s);
    std::string to_upper(const std::string &s);

    bool contains(const std::string &s, const char &value, const bool &is_value_to_lower);
    bool contains(const std::string &s, const std::string &value, const bool &is_value_to_lower);
    bool starts_with(const std::string &s, const std::string &value);
    bool ends_with(const std::string &s, const std::string &value);
    std::string replace(const std::string &s, const std::string &from, const std::string &to);
    std::string reverse(const std::string &s);
    bool is_number(const std::string &s);
}

#endif