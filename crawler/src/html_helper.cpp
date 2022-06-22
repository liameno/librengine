#include "../include/html_helper.h"

#include "../../lib/include/helper.h"

using namespace librengine::helper;

namespace html_helper {
    std::string
    get_desc(const std::string &attribute_name, const std::string &attribute_value, lxb_html_document *document) {
        auto collection = lxb_dom_collection_make(&(document)->dom_document, 16);
        lxb_dom_elements_by_attr(lxb_dom_interface_element(document->head), collection,
                                 std_string_to_lxb(attribute_name),
                                 attribute_name.length(), std_string_to_lxb(attribute_value), attribute_value.length(),
                                 true);

        const auto c_length = collection->array.length;
        std::string desc;

        for (size_t i = 0; i < c_length; i++) {
            auto element = lxb_dom_collection_element(collection, i);
            const auto content = lxb_dom_element_get_attribute(element, std_string_to_lxb("content"), 7, nullptr);

            if (content != nullptr) {
                if (desc.length() > 500) break;
                desc.append(lxb_string_to_std(content).value_or(""));
                desc.append("\n");
            }
        }

        if (c_length > 0) lxb_dom_collection_destroy(collection, true);
        return desc;
    }

    std::string compute_desc(const std::string &tag_name, lxb_html_document *document) {
        auto collection = lxb_dom_collection_make(&(document)->dom_document, 16);
        lxb_dom_elements_by_tag_name(lxb_dom_interface_element(document->body), collection, std_string_to_lxb(tag_name),
                                     tag_name.length());

        const auto c_length = collection->array.length;
        std::string desc;

        for (size_t i = 0; i < c_length; i++) {
            if (desc.length() > 500) break;

            auto element = lxb_dom_collection_element(collection, i);
            const auto text = lxb_string_to_std(
                    lxb_dom_node_text_content(lxb_dom_interface_node(element), nullptr)).value_or("");
            desc.append(text);
            desc.append("\n");
        }

        if (c_length > 0) lxb_dom_collection_destroy(collection, true);
        return desc;
    }
}