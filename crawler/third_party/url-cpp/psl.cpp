#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "psl.h"
#include "punycode.h"

namespace Url
{
    const std::string PSL::not_found = "";

    PSL::PSL(std::istream& stream)
    {
        std::string line;
        while (std::getline(stream, line))
        {
            // Only take up to the first whitespace.
            auto it = std::find_if(line.begin(), line.end(), ::isspace);
            line.resize(it - line.begin());

            // Skip blank lines
            if (line.empty())
            {
                continue;
            }

            // Skip comments
            if (line.compare(0, 2, "//") == 0)
            {
                continue;
            }

            // We know the line has at least a single character at this point
            if (line[0] == '*')
            {
                // Line is a wildcard rule
                if (line.size() <= 2 || line[1] != '.')
                {
                    throw std::invalid_argument("Wildcard rule must be of form *.<host>");
                }

                add(line, 1, 2);
            }
            else if (line[0] == '!')
            {
                // Line is an exception, take all but the !
                if (line.size() <= 1)
                {
                    throw std::invalid_argument("Exception rule has no hostname.");
                }

                add(line, -1, 1);
            }
            else
            {
                add(line, 0, 0);
            }
        }
    }

    PSL PSL::fromPath(const std::string& path)
    {
        std::ifstream stream(path);
        if (!stream.good())
        {
            std::stringstream message;
            message << "Path '" << path << "' inaccessible.";
            throw std::invalid_argument(message.str());
        }
        return PSL(stream);
    }

    PSL PSL::fromString(const std::string& str)
    {
        std::stringstream stream(str);
        return PSL(stream);
    }

    std::string PSL::getTLD(const std::string& hostname) const
    {
        return getLastSegments(hostname, getTLDLength(hostname));
    }

    std::string PSL::getPLD(const std::string& hostname) const
    {
        return getLastSegments(hostname, getTLDLength(hostname) + 1);
    }

    std::pair<std::string, std::string> PSL::getBoth(const std::string& hostname) const
    {
        size_t length = getTLDLength(hostname);
        return std::make_pair(
            getLastSegments(hostname, length),
            getLastSegments(hostname, length + 1));
    }

    size_t PSL::getTLDLength(const std::string& hostname) const
    {
        // Reversed copy of hostname
        std::string tld(hostname.rbegin(), hostname.rend());
        std::transform(tld.begin(), tld.end(), tld.begin(), ::tolower);

        while (tld.size())
        {
            auto it = levels.find(tld);
            if (it != levels.end())
            {
                return it->second;
            }

            size_t position = tld.rfind('.');
            if (position == std::string::npos || position == 0)
            {
                tld.resize(0);
            }
            else
            {
                tld.resize(position);
            }
        }

        return 1;
    }

    std::string PSL::getLastSegments(const std::string& hostname, size_t segments) const
    {
        size_t position = hostname.size();
        size_t remaining = segments;
        while (remaining != 0 && position && position != std::string::npos)
        {
            position = hostname.rfind('.', position - 1);
            remaining -= 1;
        }

        if (remaining >= 1)
        {
            return not_found;
        }

        // Return the whole string if position == std:string::npos
        size_t start = (position == std::string::npos) ? 0 : position + 1;

        std::string result(hostname, start);
        std::transform(result.begin(), result.end(), result.begin(), ::tolower);

        // Leading .'s indicate that the query had an empty segment
        if (result.size() && result[0] == '.')
        {
            std::stringstream message;
            message << "Empty segment in " << result;
            throw std::invalid_argument(message.str());
        }

        return result;
    }

    size_t PSL::countSegments(const std::string& hostname) const
    {
        size_t count = 1;
        size_t position = hostname.find('.');
        while (position != std::string::npos)
        {
            count += 1;
            position = hostname.find('.', position + 1);
        }
        return count;
    }

    void PSL::add(std::string& rule, int level_adjust, size_t trim)
    {
        // First unpunycoded
        std::string copy(rule.rbegin(), rule.rend() - trim);
        size_t length = countSegments(copy) + level_adjust;
        levels[copy] = length;

        // And now punycoded
        rule = Punycode::encodeHostname(rule);
        copy.assign(rule.rbegin(), rule.rend() - trim);
        levels[copy] = length;
    }

};
