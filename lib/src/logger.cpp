#include "logger.h"

#include <iostream>

void logger::print(const type &type_, const std::string &text, const std::string &id) {
    std::string type_s;
    std::string type_color;

    switch (type_) {
        case type::info:
            type_s = "INFO";
            type_color = cyan;
            break;
        case type::error:
            type_s = "ERROR";
            type_color = red;
            break;
    }

    std::cout << type_color << "[" << type_s << "] " << green << text << white << " [" << id << "]" << std::endl;
}
void logger::print(const std::string &text, const std::string &color) {
    std::cout << color << text << std::endl;
}