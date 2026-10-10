#include "drawing/rendering/shader.hpp"
#include "tools/devices.hpp"
#include "tools/fft.hpp"
#include <math.h>
#include <algorithm>
#include <Arduino.h>

uint32_t ShaderProcessor::FrameId = 0;
uint32_t ShaderProcessor::Time = 0;
uint16_t* ShaderProcessor::Texture = nullptr;

void ShaderProcessor::Hsv2Rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b) {
    if (h == 255) h = 254;
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

void ShaderProcessor::ShaderNone(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb){}

void ShaderProcessor::ShaderWobble(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb){

    if (shaderStrength <= 0.0f) return;

    static unsigned long lastTime = ~0UL;
    static int ix, iy, w00, w10, w01, w11;

    if (ShaderProcessor::Time != lastTime) {
        lastTime = ShaderProcessor::Time;

        const float t = (float)ShaderProcessor::Time / 1000.0f;   // assumes Time is in ms
        const float offsetY = sinf(t) * 2.f;
        const float offsetX = sinf(t / 2.0f) * 1.f;

        ix = (int)floorf(offsetX);
        iy = (int)floorf(offsetY);
        const int fx = (int)((offsetX - ix) * 256.0f + 0.5f);
        const int fy = (int)((offsetY - iy) * 256.0f + 0.5f);
        w00 = (256 - fx) * (256 - fy);
        w10 = fx * (256 - fy);
        w01 = (256 - fx) * fy;
        w11 = fx * fy;
    }

    const int w = (int)fb->GetSizeX();
    const int h = (int)fb->GetSizeY();

    const int sx = x - ix, sy = y - iy;
    const uint16_t c00 = fb->GetPixel(sx,     sy);
    const uint16_t c10 = fb->GetPixel(sx - 1, sy);
    const uint16_t c01 = fb->GetPixel(sx,     sy - 1);
    const uint16_t c11 = fb->GetPixel(sx - 1, sy - 1);

    auto blend = [&](int shift, int mask) {
        return (int)((((c00 >> shift) & mask) * w00 + ((c10 >> shift) & mask) * w10 +
                      ((c01 >> shift) & mask) * w01 + ((c11 >> shift) & mask) * w11 + 32768) >> 16);
    };

    const int r5 = blend(11, 0x1F), g6 = blend(5, 0x3F), b5 = blend(0, 0x1F);
    const uint8_t shiftedR = (r5 << 3) | (r5 >> 2);
    const uint8_t shiftedG = (g6 << 2) | (g6 >> 4);
    const uint8_t shiftedB = (b5 << 3) | (b5 >> 2);

    if (shaderStrength >= 1.0f) {
        r = shiftedR;
        g = shiftedG;
        b = shiftedB;
    } else {
        r = (uint8_t)(r * (1.0f - shaderStrength) + shiftedR * shaderStrength);
        g = (uint8_t)(g * (1.0f - shaderStrength) + shiftedG * shaderStrength);
        b = (uint8_t)(b * (1.0f - shaderStrength) + shiftedB * shaderStrength);
    }
}

void ShaderProcessor::ShaderRowShift(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb){

    static const int8_t waveTable[4] = { -2, 0, 2, 0 };

    unsigned long ticks = ShaderProcessor::Time / 50; // 100ms per step

    // Phase offset by row -> diagonal wave: row0 = 0,-1,0,1 ; row1 = 1,0,-1,0 ; row2 = 0,1,0,-1 ...
    long phase = ((long)ticks - y) % 4;
    if (phase < 0) phase += 4;

    int8_t shiftAmount = waveTable[phase];

    // Source column, wrapped so pixels scroll around instead of disappearing at the edge
    int16_t sizeX = (int16_t)fb->GetSizeX();
    int16_t srcX = x - shiftAmount;
    if (srcX < 0)          srcX += sizeX;
    else if (srcX >= sizeX) srcX -= sizeX;

    uint16_t packed = fb->GetPixel(srcX, y);
    uint8_t shiftedR, shiftedG, shiftedB;
    Devices::Display->color565to888(packed, shiftedR, shiftedG, shiftedB);

    if (shaderStrength >= 1.0f) {
        r = shiftedR;
        g = shiftedG;
        b = shiftedB;
    } else if (shaderStrength <= 0.0f) {
        // no effect, keep original
    } else {
        r = (uint8_t)(r * (1.0f - shaderStrength) + shiftedR * shaderStrength);
        g = (uint8_t)(g * (1.0f - shaderStrength) + shiftedG * shaderStrength);
        b = (uint8_t)(b * (1.0f - shaderStrength) + shiftedB * shaderStrength);
    }
}



void ShaderProcessor::ShaderRainbow(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb){

    uint8_t rainbowR, rainbowG, rainbowB;

    float gray = (r+g+b)/3.0f;

    Hsv2Rgb((( (ShaderProcessor::FrameId + x) % 64) / 64.0f) * 255, 255, gray, rainbowR, rainbowG, rainbowB);

    if (shaderStrength >= 1.0f) {
        // Full rainbow effect
        r = rainbowR;
        g = rainbowG;
        b = rainbowB;
    } else if (shaderStrength <= 0.0f) {
        // No effect, keep original colors
        // (r, g, b unchanged)
    } else {
        // Blend: result = original * (1 - strength) + rainbow * strength
        r = (uint8_t)(r * (1.0f - shaderStrength) + rainbowR * shaderStrength);
        g = (uint8_t)(g * (1.0f - shaderStrength) + rainbowG * shaderStrength);
        b = (uint8_t)(b * (1.0f - shaderStrength) + rainbowB * shaderStrength);
    }
}

void ShaderProcessor::ShaderFire(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb){

    if (r == 0 && g == 0 && b == 0){
        return;
    }

    constexpr int SIN_LUT_SIZE = 256; // power of 2 -> cheap wraparound via bitmask
    constexpr int MAX_ROWS = 64;
    constexpr int MAX_COLS = 64;

    static float sinLUT[SIN_LUT_SIZE];
    static bool  sinLUTReady = false;
    if (!sinLUTReady){
        for (int i = 0; i < SIN_LUT_SIZE; ++i){
            sinLUT[i] = sinf(i * (TWO_PI / SIN_LUT_SIZE));
        }
        sinLUTReady = true;
    }

    // Local lambdas so FastSin/FastCos stay contained to this function too
    auto FastSin = [](float radians) -> float {
        float scaled = radians * (SIN_LUT_SIZE / TWO_PI);
        int idx = (int)scaled & (SIN_LUT_SIZE - 1); // two's-complement wrap, works for negatives too
        return sinLUT[idx];
    };
    auto FastCos = [&](float radians) -> float {
        return FastSin(radians + HALF_PI);
    };

    static unsigned long cachedRowTime = 0xFFFFFFFFUL;
    static float rowBase[MAX_ROWS];       // heightFactor + turbulence3
    static bool  rowBlueFlame[MAX_ROWS];  // y > 28

    static unsigned long cachedColTime = 0xFFFFFFFFUL;
    static float colTurbulence[MAX_COLS]; // turbulence1 + turbulence2
    static float colFlicker[MAX_COLS];

    unsigned long rawTime = ShaderProcessor::Time;
    float time = rawTime * 0.001f;

    uint16_t sizeY = fb->GetSizeY();
    uint16_t sizeX = fb->GetSizeX();
    bool cachable = (sizeX <= MAX_COLS) && (sizeY <= MAX_ROWS);

    if (cachable && rawTime != cachedRowTime){
        for (uint16_t yy = 0; yy < sizeY; ++yy){
            float heightFactor = 1.0f - (float)yy * 0.03125f; // /32.0f
            float turbulence3  = FastCos(yy * 0.3f + time * 10.0f) * 0.15f;
            rowBase[yy] = heightFactor + turbulence3;
            rowBlueFlame[yy] = (yy > 28);
        }
        cachedRowTime = rawTime;
    }

    if (cachable && rawTime != cachedColTime){
        for (uint16_t xx = 0; xx < sizeX; ++xx){
            float turbulence1 = FastSin(xx * 0.2f + time * 5.0f) * 0.3f;
            float turbulence2 = FastSin(xx * 0.5f - time * 7.5f) * 0.2f;
            colTurbulence[xx] = turbulence1 + turbulence2;
            colFlicker[xx] = 0.7f + 0.3f * FastSin(time * 20.0f + xx * 0.5f);
        }
        cachedColTime = rawTime;
    }

    float fireIntensity;
    bool blueFlameEligible;

    if (cachable){
        fireIntensity = (rowBase[y] + colTurbulence[x]) * colFlicker[x];
        blueFlameEligible = rowBlueFlame[y];
    } else {
        // Fallback for buffers larger than the cache — same math as before, just with the LUT
        float heightFactor = 1.0f - (float)y * 0.03125f;
        float turbulence1 = FastSin(x * 0.2f + time * 5.0f) * 0.3f;
        float turbulence2 = FastSin(x * 0.5f - time * 7.5f) * 0.2f;
        float turbulence3 = FastCos(y * 0.3f + time * 10.0f) * 0.15f;
        float flicker = 0.7f + 0.3f * FastSin(time * 20.0f + x * 0.5f);
        fireIntensity = (heightFactor + turbulence1 + turbulence2 + turbulence3) * flicker;
        blueFlameEligible = (y > 28);
    }

    fireIntensity = std::clamp(fireIntensity, 0.0f, 1.0f);

    uint8_t fireR, fireG, fireB;

    if (fireIntensity > 0.8f) {
        float t = (fireIntensity - 0.8f) * 5.0f;
        fireR = 255;
        fireG = 255;
        fireB = (uint8_t)(255 * (1.0f - t));
    } else if (fireIntensity > 0.6f) {
        float t = (fireIntensity - 0.6f) * 5.0f;
        fireR = 255;
        fireG = (uint8_t)(255 * t);
        fireB = 0;
    } else if (fireIntensity > 0.3f) {
        float t = (fireIntensity - 0.3f) * 3.33333333f;
        fireR = 255;
        fireG = (uint8_t)(255 * t * 0.6f);
        fireB = 0;
    } else {
        float t = fireIntensity * 3.33333333f;
        fireR = (uint8_t)(255 * t * 0.5f);
        fireG = 0;
        fireB = 0;
    }

    if (blueFlameEligible && fireIntensity > 0.7f) {
        fireB = (uint8_t)std::min(255, fireB + 80);
        fireG = (uint8_t)std::min(255, fireG + 40);
    }

    float brightness = std::max(r, std::max(g, b)) * (1.0f / 255.0f);

    float rr = fireR * brightness;
    float gg = fireG * brightness;
    float bb = fireB * brightness;

    float strength = std::clamp(shaderStrength, 0.0f, 1.0f);
    r = (uint8_t)(r * (1.0f - strength) + rr * strength);
    g = (uint8_t)(g * (1.0f - strength) + gg * strength);
    b = (uint8_t)(b * (1.0f - strength) + bb * strength);
}

void ShaderProcessor::ShaderTexture(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb){
    
    // Simple UV mapping - map screen coordinates directly to texture
    if (ShaderProcessor::Texture != nullptr) {
        uint16_t textureColor = ShaderProcessor::Texture[y * CANVAS_WIDTH + x];
        uint8_t textureR, textureG, textureB;
        Devices::Display->color565to888(textureColor, textureR, textureG, textureB);
        
        // Blend with original color based on shaderStrength
        float strength = std::clamp(shaderStrength, 0.0f, 1.0f);
        r = (uint8_t)(r * (1.0f - strength) + textureR * strength);
        g = (uint8_t)(g * (1.0f - strength) + textureG * strength);
        b = (uint8_t)(b * (1.0f - strength) + textureB * strength);
    }
}


uint8_t colors_r[5] = {51, 255, 190, 255, 51};
uint8_t colors_g[5] = {255, 153, 190, 153, 255};
uint8_t colors_b[5] = {255, 190, 190, 190, 255};
    

void ShaderProcessor::ShaderTrans(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb){
    //Fuck transphobes
    float time = ShaderProcessor::Time * 0.001f;
    
    const int screenHeight = 32;
    const int transitionWidth = 3;  
    const int bandHeight = (screenHeight - (4 * transitionWidth)) / 5; 

    float scrollSpeed = 10.0f;  // Pixels per second
    int scrollOffset = (int)(time * scrollSpeed) % screenHeight;
    int yPos = (y + scrollOffset) % screenHeight;
    
    int band = 0;
    float blend = 0.0f;

    
    if (yPos < bandHeight) {
        band = 0;
        blend = 0.0f;
    } 
    else if (yPos < bandHeight + transitionWidth) {
        band = 0;
        blend = (float)(yPos - bandHeight) / transitionWidth;
    }
    else if (yPos < bandHeight * 2 + transitionWidth) {
        // Band 1
        band = 1;
        blend = 0.0f;
    }
    else if (yPos < bandHeight * 2 + transitionWidth * 2) {
        band = 1;
        blend = (float)(yPos - (bandHeight * 2 + transitionWidth)) / transitionWidth;
    }
    else if (yPos < bandHeight * 3 + transitionWidth * 2) {
        band = 2;
        blend = 0.0f;
    }
    else if (yPos < bandHeight * 3 + transitionWidth * 3) {
        band = 2;
        blend = (float)(yPos - (bandHeight * 3 + transitionWidth * 2)) / transitionWidth;
    }
    else if (yPos < bandHeight * 4 + transitionWidth * 3) {
        band = 3;
        blend = 0.0f;
    }
    else if (yPos < bandHeight * 4 + transitionWidth * 4) {
        band = 3;
        blend = (float)(yPos - (bandHeight * 4 + transitionWidth * 3)) / transitionWidth;
    }
    else {
        band = 4;
        blend = 0.0f;
    }
    
    uint8_t targetR, targetG, targetB;
    
    if (blend > 0.0f && band < 4) {
        targetR = (uint8_t)(colors_r[band] * (1.0f - blend) + colors_r[band + 1] * blend);
        targetG = (uint8_t)(colors_g[band] * (1.0f - blend) + colors_g[band + 1] * blend);
        targetB = (uint8_t)(colors_b[band] * (1.0f - blend) + colors_b[band + 1] * blend);
    } else {
        targetR = colors_r[band];
        targetG = colors_g[band];
        targetB = colors_b[band];
    }
    
    float brightness = std::max(r,std::max(g,b)) / 255.0f;  

    // Modulate the target color by the original brightness
    float rr = (targetR * brightness);
    float gg = (targetG * brightness);
    float bb = (targetB * brightness);
    
    // Blend with original color based on shader strength
    float strength = std::clamp(shaderStrength, 0.0f, 1.0f);
    r = (uint8_t)(r * (1.0f - strength) + rr * strength);
    g = (uint8_t)(g * (1.0f - strength) + gg * strength);
    b = (uint8_t)(b * (1.0f - strength) + bb * strength);
}


void ShaderProcessor::ShaderFFT(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b,
                                      ShaderType shdr, float shaderStrength, FrameBuffer *fb){

     constexpr int MAX_RADIUS = 32;
    constexpr unsigned long STEP_MS = 20;

    static int circleHue[MAX_RADIUS + 1];
    static bool initialized = false;
    static unsigned long lastStepTime = 0;

    // Adaptive range tracking
    static float runningMax = 1000.0f;  // start low so it climbs quickly
    static float runningMin = 0.0f;

    if (!initialized) {
        for (int i = 0; i <= MAX_RADIUS; ++i) circleHue[i] = -1;
        initialized = true;
        lastStepTime = ShaderProcessor::Time;
    }

    unsigned long rawTime = ShaderProcessor::Time;

    while (rawTime - lastStepTime >= STEP_MS) {
        for (int radius = MAX_RADIUS; radius > 0; --radius) {
            circleHue[radius] = circleHue[radius - 1];
        }

        int bandCount = g_fft.getBandCount();

        // Use peak energy across bands (ignoring near-silent ones), not the average
        float peak = 0.0f;
        for (int i = 0; i < bandCount; ++i) {
            int v = g_fft.getBandValue(i);
            if (v < 0) v = 0;
            if (v > 200000) v = 200000;
            if (v < 1000) continue; // ignore near-silent bins entirely for peak purposes
            if (v > peak) peak = (float)v;
        }

        // --- Adaptive normalization ---
        // Max slowly rises to track loud moments, decays slowly so it doesn't get stuck high forever
        if (peak > runningMax) {
            runningMax = peak;
        } else {
            runningMax = runningMax * 0.999f + peak * 0.001f; // slow decay toward recent levels
        }
        // Min slowly tracks the quiet floor
        if (peak < runningMin || runningMin == 0.0f) {
            runningMin = peak;
        } else {
            runningMin = runningMin * 0.995f + peak * 0.005f;
        }

        float range = std::max(runningMax - runningMin, 1.0f); // avoid div-by-zero
        float normalized = std::clamp((peak - runningMin) / range, 0.0f, 1.0f);

        constexpr float SPAWN_THRESHOLD = 0.05f;
        if (normalized >= SPAWN_THRESHOLD) {
            circleHue[0] = (int)(normalized * 255.0f);
        } else {
            circleHue[0] = -1;
        }

        lastStepTime += STEP_MS;
    }

    uint16_t sizeX = fb->GetSizeX();
    uint16_t sizeY = fb->GetSizeY();
    float cx = sizeX * 0.5f;
    float cy = sizeY * 0.5f;

    float dx = x - cx;
    float dy = y - cy;
    float dist = sqrtf(dx * dx + dy * dy);

    int ringRadius = (int)(dist + 0.5f);

    if (ringRadius > MAX_RADIUS || circleHue[ringRadius] < 0) {
        return;
    }

    float brightness = std::max(r, std::max(g, b)) / 255.0f;

    uint8_t rippleR, rippleG, rippleB;
    Hsv2Rgb((uint8_t)circleHue[ringRadius], 255, (uint8_t)(brightness * 255.0f), rippleR, rippleG, rippleB);

    float strength = std::clamp(shaderStrength, 0.0f, 1.0f);
    r = (uint8_t)(r * (1.0f - strength) + rippleR * strength);
    g = (uint8_t)(g * (1.0f - strength) + rippleG * strength);
    b = (uint8_t)(b * (1.0f - strength) + rippleB * strength);
}

void ShaderProcessor::UpdateColorByShader(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb){
    switch (shdr)
    {
    case SHADER_NONE:
        ShaderNone(x, y, r, g, b, shdr, shaderStrength, fb);
        break;
    
    case SHADER_RAINBOW:
        ShaderRainbow(x, y, r, g, b, shdr, shaderStrength, fb);
        break;
    case SHADER_FFT:
        if (g_fft.isRunning())
            ShaderFFT(x, y, r, g, b, shdr, shaderStrength, fb);
        break;

    case SHADER_FIRE:
        ShaderFire(x, y, r, g, b, shdr, shaderStrength, fb);  
        break;
    
    case SHADER_WOBBLE:
        ShaderWobble(x, y, r, g, b, shdr, shaderStrength, fb);  
        break;
    
    case SHADER_TEXTURE:
        ShaderTexture(x, y, r, g, b, shdr, shaderStrength, fb);  
        break;
    
    case SHADER_ROW_SHIFT:
        ShaderRowShift(x, y, r, g, b, shdr, shaderStrength, fb);  
        break;
    
    case SHADER_TRANS:
        ShaderTrans(x, y, r, g, b, shdr, shaderStrength, fb);  
        break;
    
    default:
        break;
    }
}


void ShaderProcessor::IncrFrame(){
    ShaderProcessor::FrameId++;
    ShaderProcessor::Time = millis();
}