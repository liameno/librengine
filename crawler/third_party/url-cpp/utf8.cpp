#include <algorithm>
#include <string>
#include <iostream>

#include "utf8.h"

namespace Url
{

    Utf8::codepoint_t Utf8::readCodepoint(
        std::string::const_iterator& it, const std::string::const_iterator& end)
    {
        Utf8::char_t current = static_cast<Utf8::char_t>(*it++);
        if (current & 0x80)
        {
            // Number of additional bytes needed
            unsigned int bytes = 0;
            // The accumulated value
            Utf8::codepoint_t result = 0;
            if (current < 0xC0)
            {
                // Invalid sequence
                throw std::invalid_argument("Low UTF-8 start byte");
            }
            else if (current < 0xE0)
            {
                // One additional byte, two bytes total, use 5 bits
                bytes = 1;
                result = current & 0x1F;
            }
            else if (current < 0xF0)
            {
                // Two additional bytes, three bytes total, use 4 bits
                bytes = 2;
                result = current & 0x0F;
            }
            else if (current < 0xF8)
            {
                // Three additional bytes, four bytes total, use 3 bits
                bytes = 3;
                result = current & 0x07;
            }
            else
            {
                throw std::invalid_argument("High UTF-8 start byte");
            }

            for (; bytes > 0; --bytes) {
                if (it == end)
                {
                    throw std::invalid_argument("UTF-8 sequence terminated early.");
                }

                current = static_cast<unsigned char>(*it++);
                // Ensure the first two bits are 10
                if ((current & 0xC0) != 0x80)
                {
                    throw std::invalid_argument("Invalid continuation byte");
                }
                result = (result << 6) | (current & 0x3F);
            }

            return result;
        }
        else
        {
            return current;
        }
    }

    std::string& Utf8::writeCodepoint(std::string& str, Utf8::codepoint_t value)
    {
        if (value > MAX_CODEPOINT)
        {
            throw std::invalid_argument("Code point too high.");
        }
        else if (value <= 0x007F)
        {
            // Just append the character itself
            str.append(1, static_cast<char>(value));
            return str;
        }

        unsigned int bytes = 0;
        if (value > 0xFFFF)
        {
            /**
             * 11110xxx + 3 bytes for 21 bits total
             *
             * We need to take bits 20-18, which 0x1C0000 masks out. These form the least
             * significant bits of this byte (so we shift them back down by 18). The 5
             * most significant bits of this byte are 11110, so we OR this result with
             * 0xF0 to get this first byte.
             *
             * The remaining bits will be consumed from the most-significant end and so
             * they must be shifted up by (32 - 18) = 14.
             */
            str.append(1, static_cast<char>(((value & 0x1C0000) >> 18) | 0xF0));
            bytes = 3;
            value <<= 14;
        }
        else if (value > 0x07FF)
        {
            /**
             * 1110xxxx + 2 bytes for 16 bits total
             *
             * We need to take bits 15-12, which 0xF000 masks out. These form the least
             * significant bits of this byte (so we shift them back down by 12). The 4
             * most significant bits of this byte are 1110, so we OR this result with
             * 0xE0 to get this first byte.
             *
             * The remaining bits will be consumed from the most-significant end and so
             * they must be shifted up by (32 - 12) = 20.
             */
            str.append(1, static_cast<char>(((value & 0xF000) >> 12) | 0xE0));
            bytes = 2;
            value <<= 20;
        }
        else
        {
            /**
             * 110xxxxx + 1 byte for 11 bits total
             *
             * We need to take bits 10-6, which 0x7C0 masks out. These form the least
             * significant bits of this byte (so we shift them back down by 6). The 3
             * most significant bits of this byte are 110, so we OR this result with
             * 0xC0 to get this first byte.
             *
             * The remaining bits will be consumed from the most-significant end and so
             * they must be shifted up by (32 - 6) = 26.
             */
            str.append(1, static_cast<char>(((value & 0x7C0) >> 6) | 0xC0));
            bytes = 1;
            value <<= 26;
        }

        /**
         * The remaining bits are to be consumed 6 at a time from the most-significant
         * end. The mask 0xFC000000 grabs these six bits, which then must be shifted down
         * by 26, and OR'd with 0x80 to produce the continuation byte.
         */
        for (; bytes > 0; --bytes, value <<= 6)
        {
            str.append(1, static_cast<char>(((value & 0xFC000000) >> 26) | 0x80));
        }

        return str;
    }

};
