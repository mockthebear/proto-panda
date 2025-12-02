#pragma once

#include "FastLED.h"
#include "config.hpp"
#include <stack>


enum LedBehavior{
    BEHAVIOR_NONE,
    BEHAVIOR_PRIDE,
    BEHAVIOR_ROTATE,
    BEHAVIOR_RANDOM_COLOR,
    BEHAVIOR_FADE_CYCLE,
    BEHAVIOR_ROTATE_FADE_CYCLE,
    BEHAVIOR_STATIC_RGB,
    BEHAVIOR_STATIC_HSV,
    BEHAVIOR_RANDOM_BLINK,
    BEHAVIOR_ICON_X,
    BEHAVIOR_ICON_I,
    BEHAVIOR_ICON_V,
    BEHAVIOR_ROTATE_SINE_V,
    BEHAVIOR_ROTATE_SINE_S,
    BEHAVIOR_ROTATE_SINE_H,
    BEHAVIOR_FADE_IN,
    BEHAVIOR_NOISE,
};


class BaseLedGroup{
    public:
    BaseLedGroup():from(0),to(0),behavior(BEHAVIOR_NONE),parameter(0),parameter2(0),parameter3(0),parameter4(0),time(0),time2(2),val(0),sum(5){};

    
    uint32_t from;
    uint32_t to;
    LedBehavior behavior;
    int parameter;
    int parameter2;
    int parameter3;
    int parameter4;

    void Update(CRGB *leds);
    void behavior_black(CRGB *leds);

    void behavior_iconX(CRGB *leds);
    void behavior_iconI(CRGB *leds);
    void behavior_iconV(CRGB *leds);

    void behavior_pride(CRGB *leds);
    void behavior_rotate(CRGB *leds);
    void behavior_randomBlink(CRGB *leds);
    void behavior_randomColor(CRGB *leds);
    void behavior_fadeCycle(CRGB *leds);
    void behavior_rotateFadeCycle(CRGB *leds);
    void behavior_StaticRgb(CRGB *leds);
    void behavior_StaticHsv(CRGB *leds);
    void behavior_sineLoop_H(CRGB *leds);
    void behavior_sineLoop_S(CRGB *leds);
    void behavior_sineLoop_V(CRGB *leds);
    void behavior_fadeIn(CRGB *leds);
    void behavior_Noise(CRGB *leds);

    uint32_t time,time2;
    int16_t val;
    int16_t sum;
};

class LedGroup : public BaseLedGroup{
    public:
        static LedGroup* PsramAllocateLedGroup(size_t len);
        LedGroup():BaseLedGroup(),m_onTween(false),m_tweenCounter(0),m_tweenMillisecondsDuration(500),m_tweening(),m_tweenBuffer(nullptr),m_tweenBufferSize(0){};

        void preallocate();
        void addTween(float rate, LedBehavior bh, int parameter = 0, int parameter2 = 0, int parameter3 = 0, int parameter4 = 0);
        void Update(CRGB *leds);
        bool m_onTween;
        uint32_t m_tweenCounter;
        uint32_t m_tweenMillisecondsDuration;
        BaseLedGroup m_tweening;
        CRGB *m_tweenBuffer;
        uint32_t m_tweenBufferSize;
};


class LedStrip { 
    public:
        LedStrip():m_ledAmount(0),m_maxBrightness(0),m_targetBrigthness(0),m_turnOnRate(0),m_currentTargetBrigtness(0),m_leds(nullptr),m_enabled(false),m_managed(false),m_gentlyTurnOn(false),m_mutex(xSemaphoreCreateMutex()){};
        bool Begin(uint16_t ledCount, uint16_t maxbrightness){
            return BeginDual(ledCount, 0, maxbrightness);
        }
        static CRGB* allocateCRGB(size_t len);
        bool BeginDual(uint16_t ledCount, uint16_t secondLedCount, uint8_t maxbrightness);
        void Update();
        
        
        void setAllColor(CRGB col);


        void setAllColorHSV(uint8_t h, uint8_t s, uint8_t v);

        CRGB *begin(){
            return m_leds;
        };

        CRGB *end(){
            return m_leds+m_ledAmount;
        }

        CRGB *getLedAddr(int i){
            return m_leds+i;
        }

        uint8_t getBrightness(){
            return m_maxBrightness;
        }
        void SetManaged(bool set){
            m_managed = set;
        }
        bool IsManaged(){
            return m_managed;
        }
        void setBrightness(uint8_t amount);

        int StackBehavior();
        int PopBehavior();

        void GentlySeBrightness(uint8_t bright, uint8_t rate = 1, uint8_t startAmount=0){
            setBrightness(startAmount); 
            m_gentlyTurnOn = true;
            m_targetBrigthness = bright;
            m_turnOnRate = rate;
            m_currentTargetBrigtness = 0;
            if (m_turnOnRate == 0){
                m_turnOnRate = 1;
            }
        }


        void Display();

        void setLedColor(int id, int r, int g, int b);
        void setSegmentColor(int id, int r, int g, int b);
        void setSegmentRange(int id, int from, int to);
        void setSegmentBehavior(int id, LedBehavior bh, int parameter = 0, int parameter2 = 0, int parameter3 = 0, int parameter4 = 0);
        void setSegmentTweenBehavior(int id, LedBehavior bh, int parameter =0, int parameter2 =0, int parameter3 =0,  int parameter4=0);
        void setSegmentTweenSpeed(int id, int parameter =0);
        int* getSegmentParameter1(int id);
        int* getSegmentParameter2(int id);
        int* getSegmentParameter3(int id);
    private:
        LedGroup m_groups[MAX_LED_GROUPS];
    
        int m_ledAmount;
        uint8_t m_maxBrightness, m_targetBrigthness, m_turnOnRate;
        uint16_t m_currentTargetBrigtness;
        CRGB *m_leds;

        bool m_enabled, m_managed, m_gentlyTurnOn;

        SemaphoreHandle_t m_mutex;
        std::stack<LedGroup*> m_behaviorStack;
};