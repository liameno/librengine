#include "str.h"

namespace librengine::str {
    std::string to_string(const std::string &value) {
        return value;
    }

    char get_first_char(const std::string &s) {
        return s.front();
    }

    char get_last_char(const std::string &s) {
        return s.back();
    }

    void remove_first_char(std::string &s) {
        s.erase(s.begin());
    }

    void remove_last_char(std::string &s) {
        s.pop_back();
    }

    std::string trim_start(const std::string &s) {
        size_t size = s.size();

        for (int i = 0; i < size; ++i) {
            char c = s[i];

            if (c != ' ') {
                return s.substr(i, (size - i));
            }
        }

        return {};
    }

    std::string trim_end(const std::string &s) {
        size_t size = s.size();

        for (size_t i = size - 1; i >= 0; --i) {
            char c = s[i];

            if (c != ' ') {
                return s.substr(0, i + 1);
            }
        }

        return {};
    }

    std::string trim(const std::string &s) {
        return trim_end(trim_start(s));
    }

    std::vector<std::string> split(const std::string &s, const std::string &delimiter) {
        std::vector<std::string> result;
        size_t delimiter_length = delimiter.length();
        size_t start = 0;

        while (true) {
            size_t end = s.find(delimiter, start);

            if (end == std::string::npos) {
                break;
            }

            std::string token = s.substr(start, end - start);
            start = end + delimiter_length;
            result.push_back(token);
        }

        result.push_back(s.substr(start));
        return result;
    }

    std::string to_lower(const std::string &s) {
        std::string result;

        for (const auto &c: s) {
            result.push_back(std::tolower(c));
        }

        return result;
    }

    std::string to_upper(const std::string &s) {
        std::string result;

        for (const auto &c: s) {
            result.push_back(std::toupper(c));
        }

        return result;
    }

    bool contains(const std::string &s, const char &value, const bool &is_value_to_lower) {
        if (is_value_to_lower) {
            return s.find(tolower(value)) != std::string::npos;
        }

        return s.find(value) != std::string::npos;
    }

    bool contains(const std::string &s, const std::string &value, const bool &is_value_to_lower) {
        if (is_value_to_lower) {
            return s.find(to_lower(value)) != std::string::npos;
        }

        return s.find(value) != std::string::npos;
    }

    bool starts_with(const std::string &s, const std::string &value) {
        size_t value_size = value.size();

        if (s.size() < value_size) {
            return false;
        }

        for (int i = 0; i < value_size; ++i) {
            if (s[i] != value[i]) {
                return false;
            }
        }

        return true;
    }

    bool ends_with(const std::string &s, const std::string &value) {
        size_t value_size = value.size();
        size_t size = s.size() - value_size;

        if (s.size() < value_size) {
            return false;
        }

        for (int i = 0; i < value_size; ++i) {
            if (s[size + i] != value[i]) {
                return false;
            }
        }

        return true;
    }

    std::string replace(const std::string &s, const std::string &from, const std::string &to) {
        std::string result = s;
        size_t start_pos = 0;

        while (true) {
            start_pos = result.find(from, start_pos);

            if (start_pos == std::string::npos) {
                break;
            }

            result.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }


        return result;
    }

    void replace_ref(std::string &s, const std::string &from, const std::string &to) {
        size_t start_pos = 0;

        while (true) {
            start_pos = s.find(from, start_pos);

            if (start_pos == std::string::npos) {
                break;
            }

            s.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    std::string reverse(const std::string &s) {
        return {s.rbegin(), s.rend()};
    }

    bool is_number(const std::string &s) {
        for (const auto &c : s) {
            if (!std::isdigit(c)) {
                return false;
            }
        }

        return true;
    }
}