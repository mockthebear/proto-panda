#pragma once
#include "config.hpp"
#include <ArduinoJson.h>
#include <string>
#include <map>
#include <FS.h>



#include "tools/psrammap.hpp"


struct SpiRamAllocator : ArduinoJson::Allocator {
  void* allocate(size_t size) override {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }

  void deallocate(void* pointer) override {
    heap_caps_free(pointer);
  }

  void* reallocate(void* ptr, size_t new_size) override {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
  }
};



class FrameRepository{
    public:
        FrameRepository(){m_bulkPercentage=0.0f;m_started=false;m_bulkFileOffset=nullptr;};
        bool Begin();

        File *takeFile();
        inline void getBulkOffsetByFrameId(int frameid, uint32_t &start, uint32_t &lenght){
          if (frameid >= m_frameCount || frameid < 0){
            Serial.printf("Tried to get frame %d but we got only %d frames\n", frameid, m_frameCount);
            start = 0;
            lenght = 0;
            return;
          }
          start = m_bulkFileOffset[frameid];
          lenght = m_bulkFileOffset[frameid+1]-m_bulkFileOffset[frameid];
        }
        void freeFile();
        void composeBulkFile();
        void displayFFATInfo();

        int getOffsetByName(std::string str){
            return m_offsets[PSRAMString(str.c_str())];
        }
        int getFrameCountByName(std::string str){
            return m_frameCountByAlias[PSRAMString(str.c_str())];
        }
        float getBulkComposingPercentage(){
          return m_bulkPercentage;
        }
    private:
        void extractModes(JsonVariant &element, bool &flip_left, bool &flip_right, int &schemeLeft, int &schemeRight);
        bool decodeFileAndWrite(const char *pathName, bool flip_left, bool flip_right, int schemeLeft, int schemeRight, int &currentOffset, int &fileIdx);
        bool loadCachedData();
        int calculateMaxFrames(JsonArray &franesJson);
        void generateCacheFile(int bulkSize);
        int m_frameCount;
        float m_bulkPercentage;
        bool m_started;
        int *m_bulkFileOffset;
        uint16_t *m_compressionBuffer;
        PSRAMIntMap m_offsets;
        PSRAMIntMap m_frameCountByAlias;
        SemaphoreHandle_t m_mutex;
        File bulkFile;
};      




