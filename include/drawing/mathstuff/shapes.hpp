#pragma once 
#include "drawing/mathstuff/primitives.hpp"

#include "esp_dsp.h"

#include <stdint.h>
#include <math.h>
#include <string.h>

class Trianglef {
public:
    Trianglef():denominator(0.0f){};
    Trianglef(const Vec2<float>& p1, const Vec2<float>& p2, const Vec2<float>& p3, uint16_t acolor){
        points.x[0] = p1.x;
        points.y[0] = p1.y;
        points.x[1] = p2.x;
        points.y[1] = p2.y;
        points.x[2] = p3.x;
        points.y[2] = p3.y;
        color = acolor;
        recalculate();
    }

    void recalculate(){
        v0X = points.x[1] - points.x[0];
        v0Y = points.y[1] - points.y[0];
        v1X = points.x[2] - points.x[0];
        v1Y = points.y[2] - points.y[0];

        float aux = ((v0X * v1Y) - (v1X * v0Y));
        if (aux != 0.0f){
            denominator = 1.0f / aux;
        }else{
            denominator = float(0.0f);
        }
    }

    void rotate(Vec2f center, float angle){

        //esp_err_t dsps_mulc_f32_ae32(const float *input, float *output, int len, float C, int step_in, int step_out);
        // x[i*step_out] = y[i*step_in]*C; i=[0..len)

        float translatedX[3];
        float translatedY[3];
       
        float aux1[3];
        float aux2[3];
        float aux3[3];

        float commonCos = cos(angle);
        float commonSin = sin(angle);
        //float translatedX = point.x - center.x;
        dsps_addc_f32(this->points.x, translatedX, 3, -center.x, 1, 1);
        //float translatedY = point.y - center.y;
        dsps_addc_f32(this->points.y, translatedY, 3, -center.y, 1, 1);
        
        // point.x = (translatedX * cos - translatedY * sin) + center.x
        dsps_mulc_f32(translatedX, aux1, 3, commonCos, 1, 1);
        dsps_mulc_f32(translatedY, aux2, 3, -commonSin, 1, 1);
        dsps_add_f32(aux1, aux2, aux3, 3, 1, 1, 1);
        dsps_addc_f32(aux3, this->points.x, 3, center.x, 1, 1);

        // point.y = (translatedX * sin + translatedY * cos) + center.y
        dsps_mulc_f32(translatedX, aux1, 3, commonSin, 1, 1);
        dsps_mulc_f32(translatedY, aux2, 3, commonCos, 1, 1);
        dsps_add_f32(aux1, aux2, aux3, 3, 1, 1, 1);
        dsps_addc_f32(aux3, this->points.y, 3, center.y, 1, 1);
    
        recalculate();
    };

    float denominator; 
    VecAligned2<float, 3> points;
    float v0X, v0Y, v1X, v1Y; 

    uint16_t color;

    bool DidIntersect(const float& x, const float& y, float& u, float& v, float& w){
        if (denominator == 0.0f){
            return false;
        }
        float v2lX = x - points.x[0];
        float v2lY = y - points.y[0];

        v = (v2lX * v1Y - v1X * v2lY) * denominator;
        w = (v0X * v2lY - v2lX * v0Y) * denominator;

        if (v < static_cast<float>(0.0) || w < static_cast<float>(0.0) || v > static_cast<float>(1.0) || w > static_cast<float>(1.0)) return false;

        u = static_cast<float>(1.0) - v - w;

        return u >= static_cast<float>(0.0f);
    }

};

template<typename T, int triangleCount> class Model {
    public:
        const int getSize(){ return triangleCount;};
        VecAligned2<T, 3 * triangleCount> points;
        T denominators[triangleCount]; 
        T v0X[triangleCount];
        T v0Y[triangleCount];
        T v1X[triangleCount];
        T v1Y[triangleCount]; 
        uint16_t color[triangleCount];

        T aux1[3 * triangleCount];
        T aux2[3 * triangleCount];
        T aux3[3 * triangleCount];

        
        void setTriangle(int i, Trianglef aux){
            if (i > triangleCount){
                return;
            }

            int localIndex = i * 3;
            points.x[localIndex  ]          = aux.points.x[0];
            points.y[localIndex  ]          = aux.points.y[0];
            points.x[localIndex+1]          = aux.points.x[1];
            points.y[localIndex+1]          = aux.points.y[1];
            points.x[localIndex+2]          = aux.points.x[2];
            points.y[localIndex+2]          = aux.points.y[2];
            color[i] = aux.color;

        }

        Trianglef getTriangle(int i){
            if (i > triangleCount){
                return Trianglef();
            }
            Trianglef aux;
            int localIndex = i * 3;
            aux.points.x[0] = points.x[localIndex];
            aux.points.y[0] = points.y[localIndex];
            aux.points.x[1] = points.x[localIndex+1];
            aux.points.y[1] = points.y[localIndex+1];
            aux.points.x[2] = points.x[localIndex+2];
            aux.points.y[2] = points.y[localIndex+2];

            aux.v0X         = v0X[i];
            aux.v0Y         = v0Y[i];
            aux.v1X         = v1X[i];
            aux.v1Y         = v1Y[i];
            aux.denominator = denominators[i];
            aux.color       = color[i];

            return aux;
        };

        bool DidIntersect(const T& x, const T& y, T& u, T& v, T& w, uint16_t &colorOut){
            for (int i=0;i<triangleCount;i++){
                if (denominators[i] == 0.0f){
                    continue;
                }
                int tringleId = i * 3;

                T v2lX = x - points.x[tringleId];
                T v2lY = y - points.y[tringleId];

                v = (v2lX * v1Y[i] - v1X[i] * v2lY) * denominators[i];
                if (v < static_cast<T>(0.0) || v > static_cast<T>(1.0)){
                    continue;
                }
                w = (v0X[i] * v2lY - v2lX * v0Y[i]) * denominators[i];

                if (w < static_cast<T>(0.0) || w > static_cast<T>(1.0)){
                    continue;
                }

                u = static_cast<T>(1.0) - v - w;

                if (u >= static_cast<T>(0.0f)){
                    colorOut = color[i];
                    return true;
                }
            }
            return false;
        }

        void recalculate(){            

            //dsps_sub_f32(input1    , input2  , *output, len          , step1, step2, step_out);
            //out[i*step_out] = input1[i*step1] - input2[i*step2]; i=[0..len)
            //For 0, 3
            //v0X[0] = points.x[0+1] - points.x[localIndex];
            //v0X[1] = points.x[3+1] - points.x[localIndex];
            //v0X[2] = points.x[6+1] - points.x[localIndex];
            dsps_sub_f32(points.x+1, points.x, v0X    , triangleCount,     3,     3, 1);
            //v0Y[i] = points.y[localIndex+1] - points.y[localIndex];
            dsps_sub_f32(points.y+1, points.y, v0Y    , triangleCount,     3,     3, 1);
            //v1X[i] = points.x[localIndex+2] - points.x[localIndex];
            dsps_sub_f32(points.x+2, points.x, v1X    , triangleCount,     3,     3, 1);
            //v1Y[i] = points.y[localIndex+2] - points.y[localIndex];
            dsps_sub_f32(points.y+2, points.y, v1Y    , triangleCount,     3,     3, 1);

            //aux1 = v0X[i] * v1Y[i]
            dsps_mul_f32(v0X, v1Y, aux1, triangleCount, 1, 1, 1);
            //aux2 = v0X[i] * v0Y[i]
            dsps_mul_f32(v1X, v0Y, aux2, triangleCount, 1, 1, 1);
            //aux1 = ((aux1[i]) - (aux2[i]));
            dsps_sub_f32(aux1, aux2, aux1    , triangleCount,     1,     1, 1);
      

            for (int i=0;i<triangleCount;i++){
                if (aux1[i] != 0.0f){
                    denominators[i] = 1.0f / aux1[i];
                }else{
                    denominators[i] = 0.0f;
                }
            }
        }

        void rotate(Vec2f center, float angle){

            //esp_err_t dsps_mulc_f32_ae32(const float *input, float *output, int len, float C, int step_in, int step_out);
            // x[i*step_out] = y[i*step_in]*C; i=[0..len)
            const int vecSize = triangleCount * 3;



            float commonCos = cos(angle);
            float commonSin = sin(angle);
            //float translatedX = point.x - center.x;
            dsps_addc_f32(this->points.x, this->points.x, vecSize, -center.x, 1, 1);
            //float translatedY = point.y - center.y;
            dsps_addc_f32(this->points.y, this->points.y, vecSize, -center.y, 1, 1);
            
            // point.x = (this->points.x * cos - this->points.y * sin) + center.x
            dsps_mulc_f32(this->points.x, aux1, vecSize, commonCos, 1, 1);
            dsps_mulc_f32(this->points.y, aux2, vecSize, -commonSin, 1, 1);
            dsps_add_f32(aux1, aux2, aux3, vecSize, 1, 1, 1);
            

            // point.y = (this->points.x * sin + this->points.y * cos) + center.y
            dsps_mulc_f32(this->points.x, aux1, vecSize, commonSin, 1, 1);
            dsps_mulc_f32(this->points.y, aux2, vecSize, commonCos, 1, 1);
            dsps_add_f32(aux1, aux2, aux1, vecSize, 1, 1, 1);
            dsps_addc_f32(aux1, this->points.y, vecSize, center.y, 1, 1);
            dsps_addc_f32(aux3, this->points.x, vecSize, center.x, 1, 1);
        
            recalculate();
        };
};

typedef Model<float, 16> Memes; 

/*
template<typename T, int N> class Shape {
    public:
        const int getSize(){ return N;};
        Triangle<T> elements[N];
};

template<typename T> class Square : public Shape<T,2>  {
public:
    Square(){};
    Square(const Vec2<T>& c1, const Vec2<T>& c2, const Vec2<T>& c3, const Vec2<T>& c4, uint16_t color=0){
        this->elements[0] = Triangle<T>(c1, c2, c3, color);
        this->elements[1] = Triangle<T>(c3, c4, c1, color);
    }

    void setColor(uint16_t color){
        this->elements[0].color = this->elements[1].color = color;
    }
};

*/