#pragma once 
#include <Arduino.h>
#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include "esp_heap_caps.h"

template <typename T>
class PSRAMAllocator {
public:
    using value_type = T;

    PSRAMAllocator() noexcept {}

    template <class U>
    PSRAMAllocator(const PSRAMAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        if (n == 0) {
            return nullptr;
        }
        void* ptr = ps_malloc(n * sizeof(T));
        if (!ptr) {
            throw std::bad_alloc();
        }
        auto Y = static_cast<T*>(ptr);;
        return Y;
    }

    void deallocate(T* p, std::size_t) noexcept {
        if (p) {
            heap_caps_free(p);
        }
    }

    bool operator!=(const PSRAMAllocator& other) const {
        return this != &other;
    }

    bool operator==(const PSRAMAllocator& other) const {
        return this == &other;
    }
};

using PSRAMString = std::basic_string<char, std::char_traits<char>, PSRAMAllocator<char>>;

struct PSRAMStringComparator {
    bool operator()(const PSRAMString& lhs, const PSRAMString& rhs) const {
        return lhs < rhs;
    }
};


using PSRAMMap = std::map<PSRAMString, PSRAMString, PSRAMStringComparator, PSRAMAllocator<std::pair<const PSRAMString, PSRAMString>>>;
using PSRAMIntMap = std::map<PSRAMString, int, PSRAMStringComparator, PSRAMAllocator<std::pair<const PSRAMString, int>>>;

template <typename T>
using PSRAMVector = std::vector<T, PSRAMAllocator<T>>;