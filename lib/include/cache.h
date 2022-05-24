#ifndef CACHE_H
#define CACHE_H

#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <list>

namespace librengine::cache {
    template<typename key, typename value>
    struct cache_node {
        template<typename u, typename v>
        cache_node(u &&key_, v &&val_) : k_(key_), v_(val_) {}

        key k_;
        value v_;
    };

    template<typename key, typename value>
    class lru {
    private:
        size_t capacity;

        std::unordered_map<key, typename std::list<cache_node<key, value>>::iterator> map;
        std::list<cache_node<key, value>> list;
    public:
        explicit lru(const size_t &capacity) {
            this->capacity = capacity;
        }

        template<typename U, typename V>
        void put(U &&key_, V &&res) {
            auto it = map.find(key_);

            if (it != map.end()) {
                it->second->v_ = std::forward<value>(res);
                list.splice(list.begin(), list, it->second);
            } else {
                list.emplace_front(key_, std::forward<value>(res));
                map.emplace(std::forward<key>(key_), list.begin());

                if (map.size() > capacity) {
                    map.erase(list.back().k_);
                    list.pop_back();
                }
            }
        }

        template<typename T>
        value *get(T &&key_) {
            auto it = map.find(std::forward<key>(key_));

            if (it != map.end()) {
                list.splice(list.begin(), list, it->second);
                return &it->second->v_;
            }

            return nullptr;
        }

        template<typename T>
        bool exists(T &&key_) {
            auto it = map.find(std::forward<key>(key_));

            if (it != map.end()) {
                list.splice(list.begin(), list, it->second);
                return true;
            }

            return false;
        }
    };
}

#endif