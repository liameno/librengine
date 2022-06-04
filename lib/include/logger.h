#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <string>

namespace logger {
    const static std::string reset    = "\033[0m";
    const static std::string black   = "\033[30m";
    const static std::string red     = "\033[31m";
    const static std::string green   = "\033[32m";
    const static std::string yellow  = "\033[33m";
    const static std::string blue    = "\033[34m";
    const static std::string magenta = "\033[35m";
    const static std::string cyan    = "\033[36m";
    const static std::string white   = "\033[37m";

    enum class type {
        info,
        error,
    };

    void print(const type &type_, const std::string &text, const std::string &id);
    void print(const std::string &text, const std::string &color = reset);
}

#endif