#pragma once 
#include "drawing/rendering/primitives.hpp"
#include "tools/config_default.hpp"

#include "esp_dsp.h"

#include <vector>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <map>


class Model;
class ModelHandler;


class PointGroups{
    public:
        PointGroups():groupCount(0),mainModel(nullptr){};
        void Translate(uint32_t group, Vec2f position);
        void Scale(uint32_t group, Vec2f center, Vec2f scaleFactors);
        void Rotate(uint32_t groupid, Vec2f center, float angle);
        void Set(uint32_t group, Vec2f position);
        Vec2f GetCenter(uint32_t group);
        bool Has(u_int32_t group);
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
        Model():triangleCount(0),aux1(nullptr),aux2(nullptr),aux3(nullptr),color(nullptr),accumulatedOperation(true),batchOperations(false){
            bones.setModel(this);
        };
        bool Begin(int sz);
        void Free();
        void Recalculate();
  
        void Reset();
        void Store();
        void CopyToRaster();

        void Rotate(Vec2f center, float angle);
        void Translate(Vec2f delta);
        void Scale(Vec2f center, Vec2f scaleFactors);

        int AddPointGroup(PointList points);
        bool hasPointGroup(uint32_t groupid);

        void TranslatePoints(uint32_t groupid, Vec2f delta);
        void RotatePoints(uint32_t groupid, Vec2f center, float angle);
        void ScalePoints(uint32_t groupid, Vec2f center, Vec2f scaleFactors);
        void SetPointsPosition(uint32_t groupid, Vec2f pos);


        void SetPointPosition(uint32_t pointId, Vec2f pos);
        void TranslatePoint(uint32_t pointid, Vec2f pos);

        int GetSize(){ return triangleCount;};
        int GetId(){ return id;};
        void SetAccumulativeOperations(bool b){
            accumulatedOperation = b;
        }
        void SetBatchOperations(bool b){
            batchOperations = b;
        }

        Vec2f GetPointGroupCenter(uint32_t pointId);

        Vec2f GetCenter(){
            return center;
        }

        int triangleCount;
        int id;
        VecAlignedCustom<float> originalPoints;
        VecAlignedCustom<float> points;
        VecAlignedCustom<float> rasterPoints;
        float *aux1;
        float *aux2;
        float *aux3;

        uint16_t *color;
        VecAligned2<float, 2> originalBoundaries;
        VecAligned2<float, 2> boundaries;
        Vec2f center;
        Vec2f originalCenter;
        PointGroups bones;
        bool accumulatedOperation;
        bool batchOperations;
        bool needRecalculate;

        void SetTriangle(int i, TriangleData aux);
        void SetTriangleF(int i, Vec2f p1, Vec2f p2, Vec2f p3, uint16_t color);

        TriangleData GetTriangle(int i);
        void RasterTriangleWithBitmap(ModelHandler *s, int i, uint8_t *targetBitmap=nullptr);
        
        inline void RasterTriangle(ModelHandler *s, int i){
            return RasterTriangleWithBitmap(s, i, nullptr);
        }
};

class ModelDict{
    public:
        Model* LoadModel(ModelData modelInfo, std::string name);
        Model* GetModelByName(std::string name){
            return m_modelsMap[name];
        }
    private:
        
        std::map<std::string, Model*> m_modelsMap;
};
extern ModelHandler g_modelHandler;
extern ModelDict g_models;