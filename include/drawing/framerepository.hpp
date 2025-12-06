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
        FrameRepository(){m_bulkPercentage=0.0f;m_started=false;};
        bool Begin();

        File *takeFile();
        void freeFile();
        void composeBulkFile();
        void displayFFATInfo();

        int getOffsetByName(std::string str){
            return m_offsets.get(str);
        }
        int getFrameCountByName(std::string str){
            return m_frameCountByAlias.get(str);
        }
        float getBulkComposingPercentage(){
          return m_bulkPercentage;
        }
    private:
        void extractModes(JsonVariant &element, bool &flip_left, bool &flip_right, int &scheme);
        bool decodeFile(const char *pathName, bool flip_left, bool flip_right, int scheme);
        bool loadCachedData();
        int calculateMaxFrames(JsonArray &franesJson);
        void generateCacheFile();
        int m_frameCount;
        float m_bulkPercentage;
        bool m_started;
        TotallyNotAMapInPsram m_offsets;
        TotallyNotAMapInPsram m_frameCountByAlias;
        SemaphoreHandle_t m_mutex;
        File bulkFile;
};      




