#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <sstream>
#include <iostream>
#include <unordered_map>

#include "../url-cpp/url.h"

#include "robots.h"

namespace Rep
{

    void Robots::strip(std::string& string)
    {
        string.erase(string.begin(), std::find_if(string.begin(), string.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
        string.erase(std::find_if(string.rbegin(), string.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), string.end());
    }

    bool Robots::getpair(std::istringstream& stream, std::string& key, std::string& value)
    {
        while (getline(stream, key))
        {
            size_t index = key.find('#');
            if (index != std::string::npos)
            {
                key.resize(index);
            }

            // Find the colon and divide it into key and value, skipping malformed lines
            index = key.find(':');
            if (index == std::string::npos)
            {
                continue;
            }

            value.assign(key.begin() + index + 1, key.end());
            key.resize(index);

            // Strip whitespace off of each
            strip(key);
            strip(value);

            // Lowercase the key
            std::transform(key.begin(), key.end(), key.begin(), ::tolower);

            return true;
        }
        return false;
    }

    Robots::Robots(const std::string& content) :
        Robots(content, "")
    {
    }

    Robots::Robots(const std::string& content, const std::string& base_url) :
        host_(Url::Url(base_url).host()),
        agents_(),
        sitemaps_(),
        default_(agents_.emplace("*", Agent(host_)).first->second)
    {
        std::string agent_name("*");
        std::istringstream input(content);
        if (content.compare(0, 3, "\xEF\xBB\xBF") == 0)
        {
            input.ignore(3);
        }
        std::string key, value;
        std::vector<std::string> group;
        bool last_agent = false;
        agent_map_t::iterator current = agents_.find("*");
        while (Robots::getpair(input, key, value))
        {
            if (key.compare("user-agent") == 0)
            {
                // Store the user agent string as lowercased
                std::transform(value.begin(), value.end(), value.begin(), ::tolower);

                if (last_agent)
                {
                    group.push_back(value);
                }
                else
                {
                    if (!agent_name.empty())
                    {
                        for (auto other : group)
                        {
                            agents_.emplace(other, current->second);
                        }
                        group.clear();
                    }
                    agent_name = value;
                    current = agents_.emplace(agent_name, Agent(host_)).first;
                }
                last_agent = true;
                continue;
            }
            else
            {
                last_agent = false;
            }

            if (key.compare("sitemap") == 0)
            {
                sitemaps_.push_back(value);
            }
            else if (key.compare("disallow") == 0)
            {
                current->second.disallow(value);
            }
            else if (key.compare("allow") == 0)
            {
                current->second.allow(value);
            }
            else if (key.compare("crawl-delay") == 0)
            {
                try
                {
                    current->second.delay(std::stof(value));
                }
                catch (const std::exception&)
                {
                    std::cerr << "Could not parse " << value << " as float." << std::endl;
                }
            }
        }

        if (!agent_name.empty())
        {
            for (auto other : group)
            {
                agents_.emplace(other, current->second);
            }
        }
    }

    const Agent& Robots::agent(const std::string& name) const
    {
        // Lowercase the agent
        std::string lowered(name);
        std::transform(lowered.begin(), lowered.end(), lowered.begin(), ::tolower);

        auto it = agents_.find(lowered);
        if (it == agents_.end())
        {
            return default_;
        }
        else
        {
            return it->second;
        }
    }

    bool Robots::allowed(const std::string& path, const std::string& name) const
    {
        return agent(name).allowed(path);
    }

    std::string Robots::str() const
    {
        std::stringstream out;
        // TODO: include sitepath info
        out << '{';
        auto begin = agents_.begin();
        auto end = agents_.end();
        if (begin != end)
        {
            out << '"' << begin->first << '"' << ": " << begin->second.str();
            ++begin;
        }
        for (; begin != end; ++begin)
        {
            out << ", \"" << begin->first << '"' << ": " << begin->second.str();
        }
        out << '}';
        return out.str();
    }

    std::string Robots::robotsUrl(const std::string& url)
    {
        return Url::Url(url)
            .setUserinfo("")
            .setPath("robots.txt")
            .setParams("")
            .setQuery("")
            .setFragment("")
            .remove_default_port()
            .str();
    }
}
