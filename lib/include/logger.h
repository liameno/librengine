#ifndef LOGGER_H
#define LOGGER_H

#include <string>

namespace logger {
    const std::string reset   = "\033[0m";
    const std::string black   = "\033[30m";
    const std::string red     = "\033[31m";
    const std::string green   = "\033[32m";
    const std::string yellow  = "\033[33m";
    const std::string blue    = "\033[34m";
    const std::string magenta = "\033[35m";
    const std::string cyan    = "\033[36m";
    const std::string white   = "\033[37m";

    enum class type {
        info,
        error,
    };

    void print(const type &type_, const std::string &text, const std::string &id);
}

#endif