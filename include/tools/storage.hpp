#pragma once
#include "config.hpp"

#include <stdint.h>
#include <Arduino.h>
#if SD_MODE == SD
#include <SD.h>
#elif SD_MODE == SD_MMC
#include <SD_MMC.h>
#endif

#include <vector>
#include <string>


class Storage{
  public:
  
    static bool Begin();
    static bool AllocatePngDecoder(bool allocateOnPsram);
    static bool DeallocatePngDecoder();
    static File getFile(const char *name);  
    static String GetFileText(const char *name);  
    static unsigned char* GetFileBinayContent(const char *name, size_t *size);  
    static uint16_t* DecodeBMP(const char *name);  
    static uint16_t* DecodePNGForBuffer(const char *name, int &rcErr);  
    static uint16_t* DecodePNG(const char *name, int &rcErr, size_t &x, size_t &y);  
    static void FreeBuffer(uint16_t* ptr);
    static std::vector<std::string> listFolder(std::string path);
    static File pngDecFile;
    static uint16_t *tmpBuffer;

};      