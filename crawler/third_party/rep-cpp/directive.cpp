#include <algorithm>
#include <locale>
#include <sstream>
#include <string>

#include "../url-cpp/url.h"

#include "directive.h"

namespace Rep
{
    Directive::Directive(const std::string& line, bool allowed)
        : expression_()
        , priority_(line.size())
        , allowed_(allowed)
    {
        if (line.find('*') == std::string::npos)
        {
            expression_.assign(line);
            return;
        }

        // Remove consecutive '*'s
        expression_.reserve(line.size());
        bool star = false;
        for (auto character : line)
        {
            if (character == '*')
            {
                if (!star)
                {
                    expression_.append(1, character);
                }
                star = true;
            }
            else
            {
                expression_.append(1, character);
                star = false;
            }
        }

        // Remove trailing '*'s
        std::string::reverse_iterator last =
            std::find_if(expression_.rbegin(), expression_.rend(),
                [](const char c) {
                    return c != '*';
                });
        expression_.erase(last.base(), expression_.end());

        // Priority is the length of the expression
        priority_ = expression_.size();
    }

    bool Directive::match(const std::string::const_iterator& e_begin,
                          const std::string::const_iterator& e_end,
                          const std::string::const_iterator& p_begin,
                          const std::string::const_iterator& p_end) const
    {
        std::string::const_iterator expression_it = e_begin;
        std::string::const_iterator path_it = p_begin;
        while (expression_it != e_end && path_it != p_end)
        {
            if (*expression_it == '*')
            {
                // Advance and recurse
                ++expression_it;
                for (; path_it != p_end; ++path_it)
                {
                    if (match(expression_it, e_end, path_it, p_end))
                    {
                        return true;
                    }
                }
                return false;
            }
            else if (*expression_it == '$')
            {
                // This check expects path to be fully consumed. But since one of the
                // criteria of being in this while loop is that we've not fully consumed
                // path, return false.
                return false;
            }
            else if (*expression_it != *path_it)
            {
                // These characters must match
                return false;
            }
            else
            {
                // Advance both by one
                ++path_it;
                ++expression_it;
            }
        }

        // Return true only if we've consumed all of the expression
        if (expression_it == e_end)
        {
            return true;
        }
        else if (*expression_it == '$')
        {
            return path_it == p_end;
        }
        else
        {
            return false;
        }
    }

    std::string Directive::str() const
    {
        std::stringstream out;
        if (allowed_)
        {
            out << "Allow: " << expression_;
        }
        else {
            out << "Disallow: " << expression_;
        }
        return out.str();
    }

    bool Directive::match(const std::string& path) const
    {
        return match(expression_.begin(), expression_.end(), path.begin(), path.end());
    }

}
