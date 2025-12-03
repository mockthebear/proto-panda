#pragma once 
#include <Arduino.h>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include "esp_heap_caps.h"


class Node {
    public:
        char* alias;
        int count;

        Node(const char* argAlias, int argCount) : count(argCount) {
            alias = (char*)heap_caps_malloc(strlen(argAlias) + 1, MALLOC_CAP_SPIRAM);
            if (alias) {
                strcpy(alias, argAlias);
            }
        };
            
        void Erase() {
            if (alias != nullptr){
                heap_caps_free(alias);
                alias = nullptr;
            }
        };
        
        bool operator<(const Node& other) const {
            if (!alias || !other.alias) return false;
            return strcmp(alias, other.alias) < 0;
        }
};

class TotallyNotAMapInPsram {
private:
    std::vector<Node> entries; //Yeah yeah, i'm not allocating a vector in the psram. I dont mind. The string is the big one
    bool is_sorted = true;

    std::vector<Node>::iterator findNode(const char* alias) {
        if (!is_sorted) {
            std::sort(entries.begin(), entries.end());
            is_sorted = true;
        } 
        Node temp(alias, 0);
        
        return std::lower_bound(entries.begin(), entries.end(), temp,
            [](const Node& a, const Node& b) {
                if (!a.alias || !b.alias) return false;
                return strcmp(a.alias, b.alias) < 0;
            });
    }
    
public:
    class iterator;
    class const_iterator;
    
    TotallyNotAMapInPsram() = default;
    
    int get(const char* alias) {
        auto it = findNode(alias);
        if (it != entries.end() && it->alias) {
            return it->count;
        }
        return 0;
    }
    
    void set(const char* alias, int count) {
        auto it = findNode(alias);
        if (it != entries.end() && it->alias) {
            it->count = count;
        } else {
            entries.emplace(it, alias, count);
            is_sorted = false;
        }
    }
    
    int incr(const char* alias, int increment = 1) {
        auto it = findNode(alias);
        
        if (it != entries.end() && it->alias) {
            it->count += increment;
            return it->count;
        } else {
            it = entries.emplace(it, alias, increment);
            is_sorted = false;
            return increment;
        }
    }

    int exists(const char* alias) {
        auto it = findNode(alias);
        if (it != entries.end() && it->alias) {
            return 1;
        }
        return 0;
    }
   
    int get(const std::string& alias) {
        return get(alias.c_str());
    }
    
    void set(const std::string& alias, int count) {
        set(alias.c_str(), count);
    }
    
    int incr(const std::string& alias, int increment = 1) {
        return incr(alias.c_str(), increment);
    }
    
    int decr(const std::string& alias, int decrement = 1) {
        return decr(alias.c_str(), decrement);
    }

    int exists(const std::string& alias) {
        return exists(alias.c_str());
    }
    
    size_t size() const {
        return entries.size();
    }
    
    bool empty() const {
        return entries.empty();
    }
    
    void clear() {
        //todo, need to propperly delete.
        entries.clear(); 
    }

    void sort() {
        if (!is_sorted) {
            std::sort(entries.begin(), entries.end());
            is_sorted = true;
        }
    }
    
    bool erase(const char* alias) {
        auto it = findNode(alias);
        if (it != entries.end() && it->alias) {

            entries.erase(it);
            return true;
        }
        return false;
    }
    
    bool erase(const std::string& alias) {
        return erase(alias.c_str());
    }
   
    
    std::vector<std::pair<const char*, int>> getAll() const {
        std::vector<std::pair<const char*, int>> result;
        result.reserve(entries.size());
        for (const auto& Node : entries) {
            result.emplace_back(Node.alias, Node.count);
        }
        return result;
    }
};
