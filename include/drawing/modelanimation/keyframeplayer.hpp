#pragma once
#include <stdint.h>
#include <map>
#include <string>
#include "drawing/rendering/primitives.hpp"
#include "drawing/rendering/model.hpp"

enum KeyframeType{
    KEYFRAME_NONE=0,
    KEYFRAME_TRANSLATE,
    KEYFRAME_ROTATE,
    KEYFRAME_SCALE,
    KEYFRAME_RESET,

    KEYFRAME_TYPE_LAST,
};

const std::map<std::string, KeyframeType> KeyframeTypeMap = {
    {"translate", KEYFRAME_TRANSLATE},
    {"rotate", KEYFRAME_ROTATE},
    {"scale", KEYFRAME_SCALE},
    {"reset", KEYFRAME_RESET},
};

class Keyframe{
    public:
    Keyframe():type(KEYFRAME_NONE),playAt(0),deltaToNext(0.0f),value(0.0f, 0.0f),center(0.0f,0.0f),ignoreInterpolation(false), dynamicCenter(false){};
    Keyframe(KeyframeType mode, uint32_t at, Vec2f val, Vec2f cntr = Vec2(0.0f, 0.0f), bool igInterp = false, bool dynamicCenter=false):type(mode),playAt(at),value(val),center(cntr),ignoreInterpolation(false), dynamicCenter(false){};

    
    KeyframeType type;
    uint32_t playAt;
    uint32_t deltaToNext;

    Vec2f value;
    Vec2f center;
    bool ignoreInterpolation, dynamicCenter;
};

class KeyframeTrack{
    public:
        KeyframeTrack(){};
        
        void Reserve();

        Model* SetResource(std::string resourceName);
        void AddKeyFrame(Keyframe kf);

        void UpdateTrack(uint32_t dt, uint32_t prevDt);
        void Reset();

        void applyTransformations(uint32_t remaining, Keyframe &nextKf, bool instant);
        

        bool usingModel;
        Model *obj;
        uint32_t pointGroup;
        uint32_t frameId;
        std::vector<int> currentFrameByType;
        std::vector<Keyframe> keyframes;
        
};


class KeyframeAnimation{
    public:
        KeyframeAnimation(){};

        void AddTrack(KeyframeTrack &t);
        void Reset();



        std::string name;
        uint32_t duration;


        uint32_t m_timeNow;
        uint32_t m_prevDt;
    
        std::vector<KeyframeTrack> m_tracks; 
        std::vector<Model*> m_models;
        
};

class KeyframePlayer{
    public:
        KeyframePlayer():m_currentlyPlaying(-1){};
        void Update(uint32_t dt);
        void PlayAnimationId(int32_t id);
        int AddKeyframeAnimation(KeyframeAnimation* kf){
            m_loadedAnimations.emplace_back(kf);
            return m_loadedAnimations.size()-1;
        }

        void begin_tmp();
    private:
        void runModelAnim(uint32_t dt);
        int32_t m_currentlyPlaying;
        std::vector<KeyframeAnimation*> m_loadedAnimations;
};