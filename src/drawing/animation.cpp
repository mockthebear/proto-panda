#include "drawing/animation.hpp"
#include "drawing/dma_display.hpp"
#include "tools/oledscreen.hpp"
#include "tools/devices.hpp"

#include "tools/storage.hpp"


#include "FS.h"
#include "SPIFFS.h"

unsigned char Animation::buffer[FILE_SIZE];

#ifdef ENABLE_HUB75_PANEL




AnimationFrameAction AnimationSequence::InterruptFrame(int pinRead){ 
    if (pinRead == 0){
        m_frame++;
        int len = m_frames.size(); 
        if (m_frame > len-1){
            m_frame = 1;
        }
        return ANIMATION_FRAME_CHANGED;
    }else{
        if (m_frame != 0){
            m_frame = 0;
            return ANIMATION_FRAME_CHANGED;
        }
        return ANIMATION_NO_CHANGE;
    }
}

AnimationFrameAction AnimationSequence::ChangeFrame(){
    int len = m_frames.size(); 
    if (m_frame >= (len-1) ){
        m_frame = 0;
        if (m_repeat == -1){
            return ANIMATION_FRAME_CHANGED;
        }
        m_repeat--;
        if (m_repeat <= 0){
            return ANIMATION_FINISHED;
        }
        return ANIMATION_FRAME_CHANGED;
    }else{
        m_frame++;
        return ANIMATION_FRAME_CHANGED;
    }
}

AnimationFrameAction AnimationSequence::Update(int m_interruptPin){
    if (m_isNew){
        m_counter = millis()+m_duration;
        m_isNew = false;
        return ANIMATION_FRAME_CHANGED;
    }
    if (m_counter <= millis()){
        m_counter = millis()+m_duration;
        if (m_interruptPin < 0){
            return ChangeFrame();
        }
        switch (m_updateMode)
        {
        case 1:
            return InterruptFrame(digitalRead(m_interruptPin));
            break;
        
        default:
            return ChangeFrame();
        }
    }
    return ANIMATION_NO_CHANGE;
 
}
int AnimationSequence::GetFrameId(){
    return m_frames[m_frame];
}

void hsv_to_rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b) {
  // Convert hue to degrees
  float hue = h / 255.0f * 360.0f;

  // Convert saturation and value to percentages
  float saturation = s / 255.0f;
  float value = v / 255.0f;

  // Calculate chroma
  float chroma = value * saturation;

  // Find the hue sector
  float hue_sector = hue / 60.0f;
  int hue_sector_int = (int)hue_sector;

  // Calculate the intermediate value x
  float x = chroma * (1.0f - fabs(fmod(hue_sector, 2.0f) - 1.0f));

  // Calculate the values of r, g, and b
  float r_temp = 0.0f, g_temp = 0.0f, b_temp = 0.0f;
  switch(hue_sector_int) {
    case 0:
      r_temp = chroma;
      g_temp = x;
      b_temp = 0;
      break;
    case 1:
      r_temp = x;
      g_temp = chroma;
      b_temp = 0;
      break;
    case 2:
      r_temp = 0;
      g_temp = chroma;
      b_temp = x;
      break;
    case 3:
      r_temp = 0;
      g_temp = x;
      b_temp = chroma;
      break;
    case 4:
      r_temp = x;
      g_temp = 0;
      b_temp = chroma;
      break;
    case 5:
      r_temp = chroma;
      g_temp = 0;
      b_temp = x;
      break;
  }

  // Calculate the final values of r, g, and b
  float m = value - chroma;
  r = (uint8_t)((r_temp + m) * 255);
  g = (uint8_t)((g_temp + m) * 255);
  b = (uint8_t)((b_temp + m) * 255);
}

void reorder_rgb(ColorMode mode, uint8_t *r, uint8_t *g, uint8_t *b){
    uint8_t auxr = *r;
    uint8_t auxb = *b;
    uint8_t auxg = *g;
    switch (mode)
    {
    case COLOR_MODE_RGB:
        break;
    case COLOR_MODE_RBG:
        *b = auxg;
        *g = auxb;
        break;
    case COLOR_MODE_GRB:
        *r = auxg;
        *g = auxr;
        break;
    case COLOR_MODE_GBR:
        *g = auxr;
        *b = auxg;
        *r = auxb;
        break;
    case COLOR_MODE_BRG:
        *b = auxr;
        *r = auxg;
        *g = auxb;
        break;
    case COLOR_MODE_BGR:
        *b = auxr;
        *r = auxb;
        break;
    default:
        break;
    }
}


void Animation::DrawFrame(File *file, int i){
    if (i == 0){
        return;
    }
    uint64_t ld = micros();
    uint64_t begin = ld;
    

    static int16_t frameId = 0;
    frameId++;    

    i--;
    
    uint32_t startPosition = i * FILE_SIZE;



    file->seek(startPosition);

    size_t rd = file->readBytes((char*)buffer, FILE_SIZE);

    if (rd != FILE_SIZE){
        Serial.printf("Failed to read %d at pos %d on file with size %d. Read just %d bytes\n", i , startPosition, file->size(), rd);
        return;
    }


    m_frameLoadDuration = micros()-ld;
    ld = micros();

    auto currentMode = m_colorMode;

    
    uint8_t r, g, b;
    uint8_t reverse = buffer[0];
    
    int byteId = 1;
    uint16_t *readBuffer = (uint16_t *)(buffer);
    
    DMADisplay::Display->startWrite();
    for (int16_t y=0;y<PANEL_HEIGHT;y++){
      for (int16_t x=0;x<PANEL_WIDTH;x++){
        uint16_t color = readBuffer[byteId++];
        //1100011100011000    = 0xC718
        //We know each color has 5 6 and 5 bits. So to check if the color is strong enough, we're using this mask that discards
        //each of 3 initial bits of each color
        //If any of the remaining bits are 1, the whole condition will give != 0 and therefore color!
        if ((color & 0x8610) != 0) { 
            OledScreen::DisplayFace[byteId] = 1;
        }else{
            OledScreen::DisplayFace[byteId] = 0;
        }
        
        

        DMADisplay::Display->color565to888(color, r, g, b);

        if (m_shader == 1 || (r == 57 && g == 121 && b == 181)){
            float gray = (r+g+b)/3.0f;
            hsv_to_rgb(  (((frameId+x)%64) / 64.0f) * 255, 255, gray, r, g, b);
        }

        reorder_rgb(currentMode, &r, &g, &b);

        DMADisplay::Display->updateMatrixDMABuffer_2((PANEL_WIDTH-1)-x, y, r, g, b);
        if (reverse&1){
           DMADisplay::Display->updateMatrixDMABuffer_2((PANEL_WIDTH)+x, y, r, g, b);
        }else{
           DMADisplay::Display->updateMatrixDMABuffer_2((PANEL_WIDTH+PANEL_WIDTH-1)-x, y, r, g, b);
        }
        
      }
    }

    DMADisplay::Display->endWrite();
    m_needFlip = true;
    m_frameDrawDuration = micros()-ld;
    m_cycleDuration =  micros()-begin;
}

bool Animation::PopAnimation(){
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    if (m_animations.size() > 0){
        xSemaphoreGive(m_mutex);
        m_animations.pop();
        return true;
    }else{
        xSemaphoreGive(m_mutex);
        return false;
    }
    
}
void Animation::MakeFlip(){
    DMADisplay::Display->flipDMABuffer();
    m_needFlip = false;
}

void Animation::SetShader(int id){
    m_shader = id;
}

void Animation::Update(File *file){
    
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    if (m_animations.size() > 0){
        auto &elem = m_animations.top();
        xSemaphoreGive(m_mutex);
        bool canPop = internalUpdate(file, elem);
        if (canPop){
            PopAnimation();
        }
    }else{
        xSemaphoreGive(m_mutex);
    }
    if (isManaged() && needFlipScreen()){
        MakeFlip();
    }
}

void Animation::setManaged(bool v){
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    m_isManaged = v;
    xSemaphoreGive(m_mutex);
}

bool Animation::internalUpdate(File *file, AnimationSequence &running){
    switch (running.Update(m_interruptPin)){
    case ANIMATION_FINISHED:
        return 1;
        break;
    case ANIMATION_FRAME_CHANGED:    
        m_lastFace = running.GetFrameId();
        if (isManaged()){
            DrawFrame(file, m_lastFace);
        }
        break;
    case ANIMATION_NO_CHANGE:
        if (m_shader == 1){
            m_lastFace = running.GetFrameId();
            if (isManaged()){
                DrawFrame(file, m_lastFace);
            }
        }
        if (m_needRedraw){
            m_needRedraw = false;
            if (isManaged()){
                DrawFrame(file, m_lastFace);
            }
        }
        break;
    }
    return false;
}

int Animation::getCurrentAnimationStorage(){
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    if (m_animations.size() > 0){
        auto &elem = m_animations.top();
        int aux = elem.m_storageId;
        xSemaphoreGive(m_mutex);
        return aux;
    }else{
        xSemaphoreGive(m_mutex);
        return -1;
    }   
}

void Animation::SetInterruptAnimation(int duration, std::vector<int> frames){
    

    AnimationSequence newSeq;
    newSeq.m_duration = max(duration,1);
    newSeq.m_frames = frames;
    newSeq.m_counter = millis()+duration;
    newSeq.m_frame = 0;
    newSeq.m_repeat = -1;
    newSeq.m_updateMode = 1;
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    m_animations.emplace(newSeq);
    xSemaphoreGive(m_mutex);
}

void Animation::SetAnimation(int duration, std::vector<int> frames, int repeatTimes, bool dropAll, int externalStorageId){
    
    if (dropAll){
        xSemaphoreTake(m_mutex, portMAX_DELAY);
        while (m_animations.size() > 0){
            m_animations.pop();
        }
        xSemaphoreGive(m_mutex);
    }

    AnimationSequence newSeq;
    newSeq.m_duration = max(duration,1);
    newSeq.m_frames = frames;
    newSeq.m_counter = 0;
    newSeq.m_frame = 0;
    newSeq.m_isNew = true;
    newSeq.m_repeat = repeatTimes;
    newSeq.m_storageId = externalStorageId;
    xSemaphoreTake(m_mutex, portMAX_DELAY);
    m_animations.emplace(newSeq);
    xSemaphoreGive(m_mutex);
}


#endif