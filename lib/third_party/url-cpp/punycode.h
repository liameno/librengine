#ifndef PUNYCODE_CPP_H
#define PUNYCODE_CPP_H

#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <limits>

#include "utf8.h"

namespace Url
{

    namespace Punycode
    {
        typedef Utf8::codepoint_t punycode_uint;

        const unsigned int BASE          = 36;
        const unsigned int TMIN          = 1;
        const unsigned int TMAX          = 26;
        const unsigned int SKEW          = 38;
        const unsigned int DAMP          = 700;
        const unsigned int INITIAL_BIAS  = 72;
        const unsigned int INITIAL_N     = 128;

        // Codepoints to their base-36 value
        const std::vector<int8_t> BASIC_TO_DIGIT = {
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1, -1,

            -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,

            -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,

            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,

            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
            -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
        };
        const std::string DIGIT_TO_BASIC = "abcdefghijklmnopqrstuvwxyz0123456789";

        // The highest codepoint in unicode
        const punycode_uint MAX_PUNYCODE_UINT = std::numeric_limits<punycode_uint>::max();
        //Utf8::MAX_CODEPOINT;
        //std::numeric_limits<punycode_uint>::max();

        /**
         * Replace utf-8-encoded str into punycode.
         */
        std::string& encode(std::string& str);

        /**
         * Create a new punycoded string from utf-8-encoded input.
         */
        std::string encode(const std::string& str);

        /**
         * Encode a hostname.
         */
        std::string encodeHostname(const std::string& hostname);

        /**
         * Replace punycoded str into utf-8-encoded.
         */
        std::string& decode(std::string& str);

        /**
         * Create a new utf-8-encoded string from punycoded input.
         */
        std::string decode(const std::string& str);

        /**
         * Decode a hostname.
         */
        std::string decodeHostname(const std::string& hostname);

        /**
         * Determine if a string needs punycoding.
         */
        bool needsPunycoding(const std::string& str);

        /**
         * Internal function for calculating bias.
         */
        punycode_uint adapt(
            punycode_uint delta, punycode_uint numpoints, bool firsttime);

    };

}

#endif
