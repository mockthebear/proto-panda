#pragma once 
#include "drawing/mathstuff/primitives.hpp"
#include "tools/config_default.hpp"

#include "esp_dsp.h"

#include <vector>
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

        u = 1.0001f - v - w;

        return u >= static_cast<float>(0.0f);
    }

};


/*
    Model does not use the triangle class. This is due data alignment. Data oriented design!
*/

class Model;

typedef std::vector<int> PointList;
class PointGroups{
    PointGroups():groupCount(0),mainModel(nullptr){};
    public:
        void Translate(uint32_t group, Vec2f position);
        void Set(uint32_t group, Vec2f position);
        uint32_t groupCount;
        Model *mainModel;
        std::vector<PointList> points;
    private:
        void setModel(Model *m){
            mainModel = m;
        }
        friend class Model;
};

class Model {
    public:
        Model():triangleCount(0),denominators(nullptr),v0X(nullptr),v0Y(nullptr),v1X(nullptr),v1Y(nullptr),aux1(nullptr),aux2(nullptr),aux3(nullptr),color(nullptr),accumulatedOperation(true),batchOperations(false){
            bones.setModel(this);
        };
        bool Begin(int sz);
        void Free();
        void Recalculate();
        void Rotate(Vec2f center, float angle);
        void Translate(Vec2f delta);
        void Reset();
        void Store();
        void Scale(Vec2f center, Vec2f scaleFactors);
        int AddPointGroup(std::vector<int> points);

        void TranslatePoints(uint32_t pointId, Vec2f delta);
        void SetPointsPosition(uint32_t pointId, Vec2f pos);

        const int GetSize(){ return triangleCount;};
        void SetAccumulativeOperations(bool b){
            accumulatedOperation = b;
        }
        void SetBatchOperations(bool b){
            batchOperations = b;
        }

        int triangleCount;
        VecAlignedCustom<float> originalPoints;
        VecAlignedCustom<float> points;
        float *denominators; 
        float *v0X;
        float *v0Y;
        float *v1X;
        float *v1Y; 
        float *aux1;
        float *aux2;
        float *aux3;

        uint16_t *color;
        VecAligned2<float, 2> boundaries;
        Vec2f center;
        PointGroups bones;
        bool accumulatedOperation;
        bool batchOperations;
        bool needRecalculate;

        void SetTriangle(int i, Trianglef aux);
        void SetTriangle(int i, Vec2f p1, Vec2f p2, Vec2f p3, uint16_t color);

        Trianglef GetTriangle(int i);
        //https://www.rose-hulman.edu/class/cs/csse351-abet/m10/triangle_fill.pdf
        //https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/rasterization-stage.html
        bool DidIntersect(const float& x, const float& y, float& u, float& v, float& w, uint16_t &colorOut);


};

class Scene {
    public:
        

        void RenderScene();
        inline bool RenderModels(const float& x, const float& y, uint16_t &colorOut);

        void addModel(Model *m){
            models.emplace_back(m);
        };


        std::vector<Model*> models;
};
