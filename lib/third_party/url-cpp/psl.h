#ifndef PSL_CPP_H
#define PSL_CPP_H

#include <istream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>

namespace Url
{

    /**
     * Find TLDs and PLDs of a hostname according to a PSL.
     */
    struct PSL
    {
        /**
         * Indicates the there is no TLD / PLD
         */
        static const std::string not_found;

        /**
         * Read a PSL from an istream.
         */
        PSL(std::istream& stream);

        PSL(): levels() { };

        PSL(const PSL& other): levels(other.levels) { }

        PSL& operator=(const PSL& other)
        {
            levels = other.levels;
            return *this;
        }

        /**
         * Read the provided path holding a set of PSL rules.
         */
        static PSL fromPath(const std::string& path);

        /**
         * Create a PSL object from a string.
         */
        static PSL fromString(const std::string& str);

        /**
         * Get just the TLD of the hostname.
         *
         * Works if the hostname is _either_ punycoded or unpunycoded, but not mixed. If
         * some segments have been appropriately punycoded and others not, it may return
         * a wrong answer. If a punycoded host is provided, a punycoded response is
         * returned. If an unpunycoded host is provided, an unpunycoded response is
         * returned.
         */
        std::string getTLD(const std::string& hostname) const;

        /**
         * Get just the PLD of the hostname.
         *
         * Works if the hostname is _either_ punycoded or unpunycoded, but not mixed. If
         * some segments have been appropriately punycoded and others not, it may return
         * a wrong answer. If a punycoded host is provided, a punycoded response is
         * returned. If an unpunycoded host is provided, an unpunycoded response is
         * returned.
         */
        std::string getPLD(const std::string& hostname) const;

        /**
         * Get the (TLD, PLD) of the hostname.
         *
         * Works if the hostname is _either_ punycoded or unpunycoded, but not mixed. If
         * some segments have been appropriately punycoded and others not, it may return
         * a wrong answer. If a punycoded host is provided, a punycoded response is
         * returned. If an unpunycoded host is provided, an unpunycoded response is
         * returned.
         */
        std::pair<std::string, std::string> getBoth(const std::string& hostname) const;
    private:
        // Mapping of a string rule to its level
        std::unordered_map<std::string, size_t> levels;

        // Return the number of segments in a hostname
        size_t countSegments(const std::string& hostname) const;

        // Return the number of segments in the TLD of the provided hostname
        size_t getTLDLength(const std::string& hostname) const;

        // Return the last `segments` segments of a hostname
        std::string getLastSegments(const std::string& hostname, size_t segments) const;

        /**
         * Add the provided host with the provided priority, trimming characters off
         * the front, and adjusting the level by the provided number.
         */
        void add(std::string& host, int level_adjust, size_t trim);
    };

}

#endif
