#include <algorithm>
#include <string>
#include <iostream>

#include "punycode.h"
#include "utf8.h"

namespace Url
{

    std::string& Punycode::encode(std::string& str)
    {
        // Pseudocode copied from https://tools.ietf.org/html/rfc3492#section-6.3
        //
        // let n = initial_n
        // let delta = 0
        // let bias = initial_bias
        punycode_uint n = INITIAL_N;
        punycode_uint delta = 0;
        punycode_uint bias = INITIAL_BIAS;
        std::string output;

        // Accumulate the non-basic codepoints
        std::vector<punycode_uint> codepoints;
        for (auto it = str.cbegin(); it != str.cend(); )
        {
            Utf8::codepoint_t value = Utf8::readCodepoint(it, str.cend());
            if (value < 0x80)
            {
                // copy them to the output in order
                output.append(1, static_cast<char>(value));
            }
            codepoints.push_back(value);
        }

        // let h = b = the number of basic code points in the input
        size_t h = output.size();
        size_t b = h;

        // copy a delimiter if b > 0
        if (b > 0)
        {
            output.append(1, '-');
        }

        // while h < length(input) do begin
        while (h < codepoints.size())
        {
            // let m = the minimum {non-basic} code point >= n in the input
            punycode_uint m = MAX_PUNYCODE_UINT;
            for (auto it = codepoints.begin(); it != codepoints.end(); ++it)
            {
                if ((*it >= n) && (*it < m))
                {
                    m = *it;
                }
            }

            // let delta = delta + (m - n) * (h + 1), fail on overflow
            if ((m - n) > ((MAX_PUNYCODE_UINT - delta) / (h + 1)))
            {
                throw std::invalid_argument("Overflow delta update.");
            }
            delta += (m - n) * (h + 1);

            // let n = m
            n = m;

            // for each code point c in the input (in order) do begin
            for (auto it = codepoints.begin(); it != codepoints.end(); ++it)
            {
                // if c < n {or c is basic} then increment delta, fail on overflow
                if (*it < n)
                {
                    if (delta == MAX_PUNYCODE_UINT)
                    {
                        throw std::invalid_argument("Overflow delta increment.");
                    }
                    ++delta;
                }

                // if c == n then begin
                if (*it == n)
                {
                    // let q = delta
                    punycode_uint q = delta;

                    // for k = base to infinity in steps of base do begin
                    for (punycode_uint k = BASE; ; k += BASE)
                    {
                        // let t = tmin if k <= bias {+ tmin}, or
                        //         tmax if k >= bias + tmax, or k - bias otherwise
                        punycode_uint t = k <= bias ? TMIN :
                                          k >= bias + TMAX ? TMAX : k - bias;

                        // if q < t then break
                        if (q < t)
                        {
                            break;
                        }

                        // output the code point for digit t + ((q - t) mod (base - t))
                        output.append(1, DIGIT_TO_BASIC[t + ((q - t) % (BASE - t))]);

                        // let q = (q - t) div (base - t)
                        q = (q - t) / (BASE - t);
                    }

                    // output the code point for digit q
                    output.append(1, DIGIT_TO_BASIC[q]);

                    // let bias = adapt(delta, h + 1, test h equals b?)
                    bias = adapt(delta, h + 1, h == b);

                    // let delta = 0
                    delta = 0;

                    // increment h
                    ++h;
                }
            }

            // increment delta and n
            ++delta;
            ++n;
        }

        str.assign(output);
        return str;
    }

    std::string Punycode::encode(const std::string& str)
    {
        std::string result(str);
        encode(result);
        return result;
    }

    std::string Punycode::encodeHostname(const std::string& hostname)
    {
        // Avoid any punycoding at all if none is needed
        if (!needsPunycoding(hostname))
        {
            return hostname;
        }

        std::string encoded;

        size_t start = 0;
        size_t end = hostname.find('.');
        while(true)
        {
            std::string segment = hostname.substr(start, end - start);
            if (needsPunycoding(segment))
            {
                encoded.append("xn--");
                encoded.append(Punycode::encode(segment));
            }
            else
            {
                encoded.append(segment);
            }

            if (end == std::string::npos)
            {
                break;
            }
            else
            {
                encoded.append(1, '.');
                start = end + 1;
                end = hostname.find('.', start);
            }
        }

        return encoded;
    }

    std::string& Punycode::decode(std::string& str)
    {
        // Pseudocode copied from https://tools.ietf.org/html/rfc3492#section-6.2
        //
        // let n = initial_n
        // let i = 0
        // let bias = initial_bias
        // let output = an empty string indexed from 0
        punycode_uint n = INITIAL_N;
        punycode_uint i = 0;
        punycode_uint bias = INITIAL_BIAS;
        std::vector<punycode_uint> codepoints;

        size_t index = str.rfind('-');
        if (index == std::string::npos)
        {
            index = 0;
        }

        // consume all code points before the last delimiter (if there is one)
        // and copy them to output, fail on any non-basic code point
        for (auto it = str.begin(); it != (str.begin() + index); ++it)
        {
            if (static_cast<unsigned char>(*it) > 127U)
            {
                throw std::invalid_argument("Argument has non-basic code points.");
            }
            codepoints.push_back(*it);
        }

        // if more than zero code points were consumed then consume one more
        //   (which will be the last delimiter)
        if (index > 0)
        {
            index += 1;
        }

        // while the input is not exhausted do begin
        for (auto it = (str.begin() + index); it != str.end(); ++it)
        {
            // let oldi = i
            // let w = 1
            punycode_uint oldi = i;
            punycode_uint w = 1;

            // for k = base to infinity in steps of base do begin
            for (punycode_uint k = BASE; ; k += BASE, ++it)
            {
                // consume a code point, or fail if there was none to consume
                if (it == str.end())
                {
                    throw std::invalid_argument("Premature termination");
                }

                // let digit = the code point's digit-value, fail if it has none
                int lookup = BASIC_TO_DIGIT[static_cast<size_t>(*it)];
                if (lookup == -1)
                {
                    throw std::invalid_argument("Invalid base 36 character.");
                }
                unsigned char digit = static_cast<unsigned char>(lookup);

                // let i = i + digit * w, fail on overflow
                if (digit > ((MAX_PUNYCODE_UINT - i) / w))
                {
                    throw std::invalid_argument("Overflow on i.");
                }
                i += digit * w;

                // let t = tmin if k <= bias {+ tmin}, or
                //         tmax if k >= bias + tmax, or k - bias otherwise
                punycode_uint t = k <= bias ? TMIN :
                                  k >= bias + TMAX ? TMAX : k - bias;

                // if digit < t then break
                if (digit < t)
                {
                    break;
                }

                // let w = w * (base - t), fail on overflow
                if (w > (MAX_PUNYCODE_UINT / (BASE - t)))
                {
                    // I believe this line is unreachable without first overflowing i.
                    // Since 'i' is updated above as i += digit * w, and w is updated as
                    // w = w * (BASE - t), we should like to keep (BASE - t) > digit to
                    // give 'w' a chance to overflow first. To keep t minimized, we must
                    // have 'bias' maximized. `bias` is driven by the 'adapt' function
                    // below.
                    //
                    // The value returned by 'adapt' increases with the input delta, and
                    // decreases with the input size. The delta is a function of the input
                    // size as well, on the order of (delta_n * input size), and
                    // legitimate delta_n values are limited to 0x10FFFF (the maximum
                    // unicode codepoint). Even setting that aside, the maximum value that
                    // adapt() can return is adapt(2 ** 32 - 1, 1, false) = 204.
                    //
                    // Using this bias, we could use the input (HERE) to get iterations:
                    //
                    //     digit = b = 1, i = 2, k = 36, t = 1, w = 35
                    //     digit = b = 1, i = 37, k = 72, t = 1, w = 1225
                    //     digit = b = 1, i = 1262, k = 108, t = 1, w = 42875
                    //     digit = b = 1, i = 44137, k = 144, t = 1, w = 1500625
                    //     digit = b = 1, i = 1544762, k = 180, t = 1, w = 52521875
                    //
                    // At this point, t now becomes TMAX (26) because k exceeds the bias
                    // (since the maximum bias is 204). As such, the minimum continuation
                    // value is 26:
                    //
                    //     digit = 0 = 26, i = 1367113512, k = 216, t = 26, w = 525218750
                    //
                    // However, the next iteration now overflows i before we can get to
                    // the w update.
                    throw std::invalid_argument("Overflow on w."); // LCOV_EXCL_LINE
                }
                w *= (BASE - t);
            }

            // let bias = adapt(i - oldi, length(output) + 1, test oldi is 0?)
            bias = adapt(i - oldi, codepoints.size() + 1, oldi == 0);

            // let n = n + i div (length(output) + 1), fail on overflow
            if ((i / (codepoints.size() + 1)) > (MAX_PUNYCODE_UINT - n))
            {
                throw std::invalid_argument("Overflow on n.");
            }
            n += i / (codepoints.size() + 1);

            // let i = i mod (length(output) + 1)
            i %= (codepoints.size() + 1);

            // insert n into output at position i
            codepoints.insert(codepoints.begin() + i, n);

            // increment i
            ++i;
        }

        std::string output;
        for (auto it = codepoints.begin(); it != codepoints.end(); ++it)
        {
            Utf8::writeCodepoint(output, *it);
        }
        str.assign(output);

        return str;
    }

    std::string Punycode::decode(const std::string& str)
    {
        std::string result(str);
        decode(result);
        return result;
    }

    std::string Punycode::decodeHostname(const std::string& hostname)
    {
        std::string unencoded;

        size_t start = 0;
        size_t end = hostname.find('.');
        while(true)
        {
            std::string segment = hostname.substr(start, end - start);
            if (segment.substr(0, 4).compare("xn--") == 0)
            {
                segment = segment.substr(4);
                unencoded.append(Punycode::decode(segment));
            }
            else
            {
                unencoded.append(segment);
            }

            if (end == std::string::npos)
            {
                break;
            }
            else
            {
                unencoded.append(1, '.');
                start = end + 1;
                end = hostname.find('.', start);
            }
        }

        return unencoded;
    }

    bool Punycode::needsPunycoding(const std::string& str)
    {
        return std::any_of(
            str.begin(),
            str.end(),
            [](char i){ return static_cast<unsigned char>(i) & 0x80; });
    }

    Punycode::punycode_uint Punycode::adapt(
        punycode_uint delta, punycode_uint numpoints, bool firsttime)
    {
        // Psuedocode from https://tools.ietf.org/html/rfc3492#section-6.1
        //
        // It does not matter whether the modifications to delta and k inside
        // adapt() affect variables of the same name inside the
        // encoding/decoding procedures, because after calling adapt() the
        // caller does not read those variables before overwriting them.
        //
        // if firsttime then let delta = delta div damp
        // else let delta = delta div 2
        delta = firsttime ? delta / DAMP : delta >> 1;

        // let delta = delta + (delta div numpoints)
        delta += (delta / numpoints);

        // let k = 0
        punycode_uint k = 0;

        // while delta > ((base - tmin) * tmax) div 2 do begin
        for (; delta > ((BASE - TMIN) * TMAX) / 2; k += BASE)
        {
            // let delta = delta div (base - tmin)
            // let k = k + base
            delta /= (BASE - TMIN);
        }

        // return k + (((base - tmin + 1) * delta) div (delta + skew))
        return k + (((BASE - TMIN + 1) * delta) / (delta + SKEW));
    }

};
