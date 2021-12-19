#ifndef DIRECTIVE_CPP_H
#define DIRECTIVE_CPP_H


namespace Rep
{

    class Directive
    {
    public:
        /**
         * The type of our priority value.
         */
        typedef size_t priority_t;

        /**
         * Default constructor disallowed.
         */
        Directive() = delete;

        /**
         * The input to this constructor must be stripped of comments
         * and trailing whitespace.
         */
        Directive(const std::string& line, bool allowed);

        /**
         * Default copy constructor.
         */
        Directive(const Directive& rhs) = default;

        /**
         * Default move constructor.
         */
        Directive(Directive&& rhs) = default;

        /**
         * The priority of the rule.
         */
        priority_t priority() const
        {
            return priority_;
        }

        /**
         * Whether or not the provided path matches. The path is
         * expected to be properly escaped.
         */
        bool match(const std::string& path) const;

        /**
         * Whether this rule is for an allow or a disallow.
         */
        bool allowed() const
        {
            return allowed_;
        }

        std::string str() const;

        /**
         * Default copy assignment operator.
         */
        Directive& operator=(const Directive& rhs) = default;

    private:
        std::string expression_;
        priority_t priority_;
        bool allowed_;

        /**
         * Return true if p_begin -> p_end matches the expression e_begin -> e_end.
         */
        bool match(const std::string::const_iterator& e_begin,
                   const std::string::const_iterator& e_end,
                   const std::string::const_iterator& p_begin,
                   const std::string::const_iterator& p_end) const;
    };

}

#endif
