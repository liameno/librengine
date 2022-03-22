#ifndef AGENT_CPP_H
#define AGENT_CPP_H

#include <vector>

#include "directive.h"

// forward declaration
namespace Url
{
    struct Url;
}

namespace Rep
{
    class Agent
    {
    public:
        /* The type for the delay. */
        typedef float delay_t;

        /**
         * Default constructor
         */
        Agent() : Agent("") {}

        /**
         * Construct an agent.
         */
        explicit Agent(const std::string& host) :
            directives_(), delay_(-1.0), sorted_(true), host_(host) {}

        /**
         * Default copy constructor.
         */
        Agent(const Agent& rhs) = default;

        /**
         * Default move constructor.
         */
        Agent(Agent&& rhs) = default;

        /**
         * Add an allowed directive.
         */
        Agent& allow(const std::string& query);

        /**
         * Add a disallowed directive.
         */
        Agent& disallow(const std::string& query);

        /**
         * Set the delay for this agent.
         */
        Agent& delay(delay_t value) {
            delay_ = value;
            return *this;
        }

        /**
         * Return the delay for this agent.
         */
        delay_t delay() const { return delay_; }

        /**
         * A vector of the directives, in priority-sorted order.
         */
        const std::vector<Directive>& directives() const;

        /**
         * Return true if the URL (either a full URL or a path) is allowed.
         */
        bool allowed(const std::string& path) const;

        std::string str() const;

        /**
         * Default copy assignment operator.
         */
        Agent& operator=(const Agent& rhs) = default;

    private:
        bool is_external(const Url::Url& url) const;

        mutable std::vector<Directive> directives_;
        delay_t delay_;
        mutable bool sorted_;
        std::string host_;
    };
}

#endif
