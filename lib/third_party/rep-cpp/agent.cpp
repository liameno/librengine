#include <algorithm>
#include <iomanip>
#include <sstream>

#include "../url-cpp/url.h"

#include "agent.h"
#include "directive.h"

namespace
{
    std::string escape_url(Url::Url& url)
    {
        return url.defrag().escape().fullpath();
    }

    std::string trim_front(const std::string& str, const char chr)
    {
        auto itr = std::find_if(str.begin(), str.end(),
                       [chr](const char c) {return c != chr;});
        return std::string(itr, str.end());
    }
}

namespace Rep
{
    Agent& Agent::allow(const std::string& query)
    {
        Url::Url url(query);
        // ignore directives for external URLs
        if (is_external(url))
        {
            return *this;
        }
        // leading wildcard?
        if (query.front() == '*')
        {
            Url::Url trimmed(trim_front(query, '*'));
            directives_.push_back(Directive(escape_url(trimmed), true));
        }
        directives_.push_back(Directive(escape_url(url), true));
        sorted_ = false;
        return *this;
    }

    Agent& Agent::disallow(const std::string& query)
    {
        if (query.empty())
        {
            // Special case: "Disallow:" means "Allow: /"
            directives_.push_back(Directive(query, true));
        }
        else
        {
            Url::Url url(query);
            // ignore directives for external URLs
            if (is_external(url))
            {
                return *this;
            }
            // leading wildcard?
            if (query.front() == '*')
            {
                Url::Url trimmed(trim_front(query, '*'));
                directives_.push_back(Directive(escape_url(trimmed), false));
            }
            directives_.push_back(Directive(escape_url(url), false));
        }
        sorted_ = false;
        return *this;
    }

    const std::vector<Directive>& Agent::directives() const
    {
        if (!sorted_)
        {
            std::sort(directives_.begin(), directives_.end(),
                [](const Directive& a, const Directive& b) {
                    return b.priority() < a.priority();
                });
            sorted_ = true;
        }
        return directives_;
    }

    bool Agent::allowed(const std::string& query) const
    {
        Url::Url url(query);
        if (is_external(url))
        {
            return false;
        }
        std::string path(escape_url(url));

        if (path.compare("/robots.txt") == 0)
        {
            return true;
        }

        for (const auto& directive : directives())
        {
            if (directive.match(path))
            {
                return directive.allowed();
            }
        }
        return true;
    }

    std::string Agent::str() const
    {
        std::stringstream out;
        if (delay_ > 0)
        {
            out << "Crawl-Delay: " << std::setprecision(3) << delay_ << ' ';
        }
        out << '[';
        const auto& d = directives();
        auto begin = d.begin();
        auto end = d.end();
        if (begin != end)
        {
            out << "Directive(" << begin->str() << ')';
            ++begin;
        }
        for (; begin != end; ++begin)
        {
            out << ", Directive(" << begin->str() << ')';
        }
        out << ']';
        return out.str();
    }

    bool Agent::is_external(const Url::Url& url) const
    {
        return !host_.empty() && !url.host().empty() && url.host() != host_;
    }
}
