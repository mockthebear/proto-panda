#pragma once

#include <stdint.h>

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