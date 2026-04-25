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
    Keyframe(KeyframeType mode, uint32_t at, Vec2f val, Vec2f cntr = Vec2(0.0f, 0.0f), bool igInterp = false, bool dynamicCenterP=false):type(mode),playAt(at),value(val),center(cntr),ignoreInterpolation(igInterp), dynamicCenter(dynamicCenterP){};


    static Keyframe KeyFrameMaker(int mode, uint32_t at, Vec2f val, Vec2f cntr = Vec2(0.0f, 0.0f), bool igInterp = false, bool dynamicCenterP=false);
    
    KeyframeType type;
    uint32_t playAt;
    uint32_t deltaToNext;

    Vec2f value;
    Vec2f center;
    bool ignoreInterpolation, dynamicCenter;
};

class KeyframeTrack{
    public:
        KeyframeTrack(){Reserve();};
        
        void Reserve();

        bool SetResource(std::string resourceName);
        int AddKeyFrame(Keyframe kf);

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
        
        void AddTrack(KeyframeTrack t);
        void AddModelReference(Model *m);
        void Reset();
        void SetId(uint32_t i){
            idx = i;
        };

        uint32_t GetId(){
            return idx;
        }


        std::string name;
        uint32_t duration;

        uint32_t idx;
        uint32_t m_prevDt;
    
        std::vector<KeyframeTrack> m_tracks; 
        std::vector<Model*> m_models;
        
};

class KeyframePlayer{
    public:
        KeyframePlayer():m_currentlyPlaying(-1),m_extraDtForNextFrame(0),m_finishedAnimationInThisFrame(false){};
        bool Update(uint32_t dt);
        void PlayAnimationId(int32_t id, bool restartAnimation=true);
        void ResetCurrentAnimation();
        void ResetSpecificAnimation(int id);
        int32_t GetCurrentPlaying(){
            return m_currentlyPlaying;
        }
        int AddKeyframeAnimation(KeyframeAnimation* kf){
            kf->Reset();
            m_loadedAnimations.emplace_back(kf);
            return m_loadedAnimations.size()-1;
        }

        KeyframeAnimation* NewKeyframeAnimation(uint32_t duration);

    private:
        void runModelAnim(uint32_t dt);
        int32_t m_currentlyPlaying;
        uint32_t m_extraDtForNextFrame;
        bool m_finishedAnimationInThisFrame;
        std::vector<KeyframeAnimation*> m_loadedAnimations;
};

extern KeyframePlayer g_kf;