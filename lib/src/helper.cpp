#include "../include/helper.h"

#include <lexbor/html/html.h>
#include <optional>
#include <thread>

namespace librengine::helper {
    size_t compute_time() {
        return time(nullptr);
    }

    std::optional<std::string> lxb_string_to_std(const lxb_char_t *s) {
        if (s == nullptr) return std::nullopt;
        return (const char *)s;
    }
    lxb_char_t *std_string_to_lxb(const std::string &s) {
        return (lxb_char_t *)s.c_str();
    }

    std::optional<lxb_html_document*> parse_html(const std::string &response) {
        auto parser = lxb_html_parser_create();
        auto status = lxb_html_parser_init(parser);

        if (status != LXB_STATUS_OK) return std::nullopt;
        auto document = lxb_html_parse(parser, std_string_to_lxb(response), response.length() - 1);
        lxb_html_parser_destroy(parser);

        if (document == nullptr) return std::nullopt;
        return document;
    }
}