#include <string>
#include <lexbor/html/html.h>
#include <optional>

#include "config.h"
#include "typesense.h"
#include "http.h"

#ifndef HELPER_H
#define HELPER_H

namespace helper {
    size_t compute_time();

    std::optional<std::string> lxb_string_to_std(const lxb_char_t *s);
    lxb_char_t *std_string_to_lxb(const std::string &s);

    std::optional<lxb_html_document*> parse_html(const std::string &response);
}

#endif