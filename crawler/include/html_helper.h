#ifndef HTML_HELPER_H
#define HTML_HELPER_H

#include <string>
#include <lexbor/html/html.h>

namespace html_helper {
    std::string get_desc(const std::string &attribute_name, const std::string &attribute_value, lxb_html_document *document);
    std::string compute_desc(const std::string &tag_name, lxb_html_document *document);
}

#endif