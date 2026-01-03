#pragma once 

#include <vector>
#include <stack>
#include <stdint.h>
#include <FS.h>
#include "config.hpp"
#include "tools/psrammap.hpp"
#include "drawing/renderer.hpp"


enum ColorMode{
    COLOR_MODE_RGB,
    COLOR_MODE_RBG,
    COLOR_MODE_GRB,
    COLOR_MODE_GBR,
    COLOR_MODE_BRG,
    COLOR_MODE_BGR,
    
};

#ifdef ENABLE_HUB75_PANEL

enum AnimationFrameAction{
    ANIMATION_NO_CHANGE,
    ANIMATION_FRAME_CHANGED,
    ANIMATION_FINISHED,
};

class AnimationSequence{
    public:
        AnimationSequence():m_duration(2500),m_frame(0),m_counter(0),m_repeat(-1),m_updateMode(0),m_storageId(-1),m_isNew(true){}
        PSRAMVector<int> m_frames;
        AnimationFrameAction Update(int m_interruptPin);
        inline int GetFrameId();
        int m_duration;
        int m_frame;
        int m_counter;
        int m_repeat;
        int m_updateMode;
        int m_storageId;
        bool m_isNew;
    private:
        AnimationFrameAction ChangeFrame();
        AnimationFrameAction InterruptFrame(int pinRead);
    
};



class Animation{
    public:
        Animation():m_animations(),m_shader(0),m_lastFace(0),m_interruptPin(-1),m_colorMode(COLOR_MODE_RGB),m_needFlip(false),m_isManaged(true),m_needRedraw(false),m_onBlankScreen(false),m_frameDrawDuration(0),m_frameLoadDuration(0),m_cycleDuration(0),m_mutex(xSemaphoreCreateMutex()){};

        void Update(File *file);

        void SetAnimation(int duration, std::vector<int> frames, int repeatTimes, bool dropAll, int externalStorageId=-1);
        void SetInterruptAnimation(int duration, std::vector<int> frames);
        void SetInterruptPin(int pin){
            m_interruptPin = pin;
        }
        void DrawFrame(File *file, int i);
        void DrawCurrentFrame(File *file){
            DrawFrame(file, m_lastFace);
        }

        bool PopAnimation();
        void MakeFlip();
        void SetShader(int id);

        void setColorMode(ColorMode mode){
            m_colorMode = mode;
            m_needRedraw = true;
        };

        bool needFlipScreen(){
            return m_needFlip;
        };
        void setManaged(bool v);
        bool isManaged(){
            return m_isManaged;
        }
        int getCurrentFace(){
            return m_lastFace;
        }

        int getCurrentAnimationStorage();

        float getFps(){
            return 1000000.0f/(float)m_cycleDuration;
        }

        int getAnimationStackSize(){
            return m_animations.size();
        }

        void setRainbowShader(bool enabled){
            if (enabled){
                m_shader = true;
            }else{
                m_shader = false;
            }
        }

        static unsigned char buffer[FILE_SIZE];

        uint32_t getDrawDuration() { return m_frameDrawDuration;};
        uint32_t getLoadDuration() { return m_frameLoadDuration;};
    private:
        std::stack<AnimationSequence> m_animations;
        RenderEngine rend;
        bool internalUpdate(File *file, AnimationSequence &seq);
        int m_shader;
        int m_lastFace;
        int m_interruptPin;
        ColorMode m_colorMode;
        bool m_needFlip;
        bool m_isManaged;
        bool m_needRedraw;
        bool m_onBlankScreen;
        
        uint64_t m_frameDrawDuration;
        uint64_t m_frameLoadDuration;
        uint64_t m_cycleDuration;
        SemaphoreHandle_t m_mutex;
        
};


extern Animation g_animation;
#else
class Animation{
    public:
        Animation(){};

        void Update(File *file){}

        void SetAnimation(int duration, std::vector<int> frames, int repeatTimes, bool dropAll, int externalStorageId=-1){}
        void SetInterruptAnimation(int duration, std::vector<int> frames){}
        void SetInterruptPin(int pin){       }
        void DrawFrame(File *file, int i){}
        void DrawCurrentFrame(File *file){}

        bool PopAnimation(){return false;}
        void MakeFlip(){}
        void SetShader(int id){}

        void setColorMode(ColorMode mode){
           
        };

        bool needFlipScreen(){
            return 0;
        };
        void setManaged(bool v){}
        bool isManaged(){
            return 0;
        }
        int getCurrentFace(){
            return 0;
        }

        int getCurrentAnimationStorage(){ return 0;}
        float getFps(){ return 0; }
        int getAnimationStackSize(){ return 0; }

        void setRainbowShader(bool enabled){}
        static unsigned char buffer[FILE_SIZE];
        uint32_t getDrawDuration() { return 0;};
        uint32_t getLoadDuration() { return 0;};
};
extern Animation g_animation;
#endif