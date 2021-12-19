#ifndef STR_IMPL_H
#define STR_IMPL_H

#include "str.h"

namespace librengine::str {
    template<typename T>
    typename std::enable_if<false == std::is_convertible<T, std::string>::value, std::string>::type
    to_string(T const &value) {
        return std::to_string(value);
    }

    template<typename T>
    typename std::enable_if<false == std::is_convertible<T, std::vector<std::string>>::value, std::string> ::type
    to_string(std::vector<T> const &values) {
        std::string result;

        for (auto value: values) {
            result.append(value);
        }

        return result;
    }

    template<typename T>
    bool contains(const std::vector<T> &values, const T value, const bool &is_value_to_lower) {
        auto end = values.end();

        if (is_value_to_lower) {
            return std::find(values.begin(), end, to_lower(value)) != end;
        }

        return std::find(values.begin(), end, value) != end;
    }

    template<typename... T>
    std::string join(const std::string &separator, const std::vector<T> &... values) {
        std::vector<std::string> args;
        std::string result;

        using unused = int[];
        (void) unused{0, (args.push_back(to_string(values)), 0)...};

        for (const auto &s: args) {
            result.append(s);
            result.append(separator);
        }

        return result;
    }

    template<typename... T>
    std::string format(const std::string &s, const T &... values) {
        std::vector<std::string> args;
        std::string result = s;

        using unused = int[];
        (void) unused{0, (args.push_back(to_string(values)), 0)...};

        char open = '{';
        char close = '}';
        bool is_open = false;
        uint index = 0;
        uint index_length = 0;
        uint start = 0;

        for (uint i = 0; i < result.length(); ++i) {
            char c = result.at(i);

            if (c == '\\') {
                i += 1;
            } else if (c == open) {
                is_open = true;
                start = i;
            } else if (c == close) {
                uint one = result.length();
                result.erase(start, i + 1 - start);

                if (index_length > 0) {
                    result.insert(start, args.at(index));
                }

                uint two = result.length();

                i -= one - two;
                is_open = false;
                index = 0;
                index_length = 0;
            } else if (is_open) {
                int n = c - '0';

                if (n < 0 || n > 9) {
                    continue;
                }

                index = (index * 10) + n;
                ++index_length;
            }
        }

        return result;
    }
}

#endif