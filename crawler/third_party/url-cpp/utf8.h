#ifndef UTF8_CPP_H
#define UTF8_CPP_H

#include <stdexcept>
#include <string>
#include <vector>

namespace Url
{

    /**
     * Work between unicode code points and their UTF-8-encoded representation.
     */
    struct Utf8
    {
        /**
         * The type we use to represent Unicode codepoints.
         */
        typedef uint32_t codepoint_t;

        /**
         * The type we use when talking about the integral value of bytes.
         */
        typedef unsigned char char_t;

        /**
         * The highest allowed codepoint.
         */
        static const codepoint_t MAX_CODEPOINT = 0x10FFFF;

        /**
         * Consume up to the last byte of the sequence, returning the codepoint.
         */
        static codepoint_t readCodepoint(
            std::string::const_iterator& it, const std::string::const_iterator& end);

        /**
         * Write a codepoint to the provided string.
         */
        static std::string& writeCodepoint(std::string& str, codepoint_t value);

        /**
         * Return the first codepoint stored in the provided string.
         */
        static codepoint_t toCodepoint(const std::string& str)
        {
            auto it = str.begin();
            return readCodepoint(it, str.end());
        }

        /**
         * Get a string with the provided codepoint.
         */
        static std::string fromCodepoint(codepoint_t value)
        {
            std::string str;
            writeCodepoint(str, value);
            return str;
        }

        /**
         * Return all the codepoints in the string.
         */
        static std::vector<codepoint_t> toCodepoints(const std::string& str)
        {
            std::vector<codepoint_t> result;
            for (auto it = str.begin(); it != str.end(); )
            {
                result.push_back(readCodepoint(it, str.end()));
            }
            return result;
        }

        /**
         * Create a string from a vector of codepoints.
         */
        static std::string fromCodepoints(const std::vector<codepoint_t>& points)
        {
            std::string result;
            for (auto it = points.begin(); it != points.end(); ++it)
            {
                writeCodepoint(result, *it);
            }
            return result;
        }

    };

}

#endif
