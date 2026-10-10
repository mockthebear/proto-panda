#pragma once
#include "drawing/rendering/primitives.hpp"
#include "drawing/rendering/shadertypes.hpp"
#include "drawing/framebuffer.hpp"
#include "tools/config_default.hpp"


class ShaderProcessor{
    private:
        static void ShaderNone(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrenght, FrameBuffer *fb);
        static void ShaderRainbow(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrenght, FrameBuffer *fb);
        static void ShaderRowShift(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrenght, FrameBuffer *fb);
        static void ShaderFire(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb);
        static void ShaderTexture(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb);
        static void ShaderTrans(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb);
        static void ShaderFFT(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb);
        static void ShaderWobble(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrength, FrameBuffer *fb);

        static uint32_t FrameId;
        static uint32_t Time;
        static uint16_t* Texture;
    public:
    static void Hsv2Rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b);
        static void SetTextureAddr(uint16_t *addr){
            Texture = addr;
        }
        static void UpdateColorByShader(int16_t &x, int16_t &y, uint8_t &r, uint8_t &g, uint8_t &b, ShaderType shdr, float shaderStrenght, FrameBuffer *fb);
        static void IncrFrame();
};