#include <algorithm>
#include <string>
#include <vector>
#include <regex>

namespace librengine {
    std::string to_string(const std::string &value) { //for format (str_impl.h)
        return value;
    }

    char get_first(const std::string &s) {
        return s.front();
    }
    char get_last(const std::string &s) {
        return s.back();
    }

    void remove_first(std::string &s) {
        s.erase(s.begin());
    }
    void remove_last(std::string &s) {
        s.pop_back();
    }

    void trim_start(std::string &s) {
        static const auto lambada_space = [](const char &c) { return std::isspace(c); };
        auto first = std::find_if_not(s.begin(), s.end(), lambada_space);
        s.erase(s.begin(), first);
    }
    void trim_end(std::string &s) {
        static const auto lambada_space = [](const char &c) { return std::isspace(c); };
        auto last = std::find_if_not(s.rbegin(), s.rend(), lambada_space);
        s.erase(last.base(), s.end());
    }
    void trim(std::string &s) {
        trim_start(s);
        trim_end(s);
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

    void to_lower(std::string &s) {
        for (auto &c : s) {
            c = std::tolower(c);
        }
    }
    void to_upper(std::string &s) {
        for (auto &c : s) {
            c = std::toupper(c);
        }
    }
    std::string to_lower_copy(const std::string &s) {
        std::string result = s;
        to_lower(result);
        return result;
    }
    std::string to_upper_copy(const std::string &s) {
        std::string result = s;
        to_upper(result);
        return result;
    }

    bool contains(const std::string &s, const char &value) {
        return s.find(value) != std::string::npos;
    }
    bool contains(const std::string &s, const std::string &value) {
        return s.find(value) != std::string::npos;
    }

    bool starts_with(const std::string &s, const std::string &v) {
        const size_t v_size = v.size();
        const size_t s_size = s.size();

        if (s_size < v_size) return false;

        for (int i = 0; i < v_size; ++i) {
            if (s[i] != v[i]) {
                return false;
            }
        }

        return true;
    }
    bool ends_with(const std::string &s, const std::string &v) {
        const size_t v_size = v.size();
        const size_t s_size = s.size();
        const size_t size = s_size  - v_size;

        if (s_size < v_size) return false;

        for (int i = 0; i < v_size; ++i) {
            if (s[size + i] != v[i]) {
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

    size_t find_end(const std::string &s, const std::string &v) {
        auto s_length = s.length();
        auto v_length = v.length();

        if (v_length > s_length) return std::string::npos;
        return s.find(v, s_length - v_length);
    }
    bool replace(std::string &s, const std::string &from, const std::string &to) {
        if (from.length() > s.length()) return false;

        size_t start_pos = 0;
        bool result = false;

        while (true) {
            start_pos = s.find(from, start_pos);

            if (start_pos == std::string::npos) {
                break;
            }

            result = true;
            s.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }

        return result;
    }
    bool replace(std::string &s, size_t start_pos, const std::string &from, const std::string &to) {
        if (from.length() > s.length()) return false;

        bool result = false;

        while (true) {
            start_pos = s.find(from, start_pos);

            if (start_pos == std::string::npos) {
                break;
            }

            result = true;
            s.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }

        return result;
    }
    std::string replace_copy(const std::string &s, const std::string &from, const std::string &to) {
        std::string result = s;
        replace(result, from, to);
        return result;
    }
    std::string replace_copy(const std::string &s, size_t start_pos, const std::string &from, const std::string &to) {
        std::string result = s;
        replace(result, start_pos, from, to);
        return result;
    }
    bool replace_end(std::string &s, const std::string &from, const std::string &to) {
        auto s_length = s.length();
        auto from_length = from.length();

        if (from_length > s_length) return false;
        return replace(s, s_length - from_length, from, to);
    }
    bool replace_end(std::string &s, const size_t &start, const std::string &from, const std::string &to) {
        if (from.length() > s.length()) return false;
        auto found = find_end(s, from);

        if (found != std::string::npos && found >= start) {
            return replace_end(s, from, to);
        }

        return false;
    }
    std::string replace_end_copy(const std::string &s, const std::string &from, const std::string &to) {
        std::string result = s;
        replace_end(result, from, to);
        return result;
    }
    std::string replace_end_copy(const std::string &s, size_t start_pos, const std::string &from, const std::string &to) {
        std::string result = s;
        replace_end(result, start_pos, from, to);
        return result;
    }

    std::string reverse(const std::string &s) {
        return {s.rbegin(), s.rend()};
    }
    void remove_special_chars(std::string &s) {
        static auto special_char_lambada = [](const char &c) { return !std::isalpha(c) && !std::isdigit(c); };
        s.erase(std::remove_if(s.begin(), s.end(), special_char_lambada), s.end());
    }
    void remove_html_tags(std::string &html) {
        std::regex regex(R"(<\/?(\w+)(\s+\w+=(\w+|"[^"]*"|'[^']*'))*(( |)\/|)>)"); //<[^<>]+>
        html = regex_replace(html, regex, "");
    }
}