#pragma once

#include <stdint.h>
#include <vector>
#include "esp_heap_caps.h"

template<typename T> class Vec2{
    public:
    Vec2():x(0),y(0){};
    Vec2(T x1, T y2):x(x1),y(y2){};
    T x;
    T y;
};


template<typename T> class Vec3 : public Vec2<T>{
    public:
    Vec3():Vec2<T>(),z(0){};
    Vec3(T x1, T y1, T z1):Vec2<T>(x1, y1),z(z1){};
    T z;
};


template<typename T> class VecAlignedCustom{
    public:
    VecAlignedCustom():x(nullptr),y(nullptr){};
    void Allocate(int size){
        x = (T*)heap_caps_malloc(size*2 * sizeof(T), MALLOC_CAP_SPIRAM);   
        y = x+size;   
    };
    void Deallocate(){
        if (x == nullptr){
            return;
        }
        heap_caps_free(x);
        x = y = nullptr;
    };
    T *x;
    T *y;
};


template<typename T, int N> class VecAligned2{
    public:
    VecAligned2(){};
    T x[N];
    T y[N];
};



typedef Vec2<float> Vec2f;
typedef Vec3<float> Vec3f;
typedef Vec2<int> Vec2i;
typedef Vec3<int> Vec3i;

class TriangleData{
    public:
        Vec2f p1;
        Vec2f p2;
        Vec2f p3;
        uint16_t color;
};

class ModelData{
    public:
        std::vector<float> x;
        std::vector<float> y;
        std::vector<uint16_t> color;
};

typedef std::vector<int> PointList;
