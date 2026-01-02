#include "tools/storage.hpp"
#include "tools/logger.hpp"
#include "drawing/dma_display.hpp"
#include <SD.h>
#include <PNGdec.h>
#include "SPI.h"
#include <string.h>

File Storage::pngDecFile;
uint16_t* Storage::tmpBuffer;



bool Storage::Begin(){
  SPI.setFrequency(SPI_MAX_CLOCK);
    if(!SD.begin(SPI_CS, SPI, SPI_MAX_CLOCK, "/sd", 10)){
        return false;
    }   
    return true;
}

File Storage::getFile(const char *name){
  File myFile = SD.open(name);
  return myFile;
}  

String Storage::GetFileText(const char *name){
  File myFile = getFile(name);
  if (!myFile) {
    return "";
  }

  size_t fileSize = myFile.size();
  
  char *buffer = (char *)ps_malloc(fileSize + 1);
  if (buffer == nullptr){
    return "OOM";
  }

  size_t bytesRead = myFile.readBytes(buffer, fileSize);
  
  buffer[bytesRead] = '\0';

  myFile.close();
  
  String fileContent = String(buffer);
  heap_caps_free(buffer);

  return fileContent;
}

std::vector<std::string> Storage::listFolder(const char *name){
  File dir = SD.open(name);
  std::vector<std::string> res;
  while (true) {
    File entry = dir.openNextFile();
    if (! entry) {
      break;
    }
    if (!entry.isDirectory()) {
      res.emplace_back(entry.name());
    }
    entry.close();
  }
  return res;
}


unsigned char* Storage::GetFileBinayContent(const char *name, size_t *size){
  File myFile = getFile(name); // Open the file
  if (!myFile) {
    Logger::Error("Oh no, no file %s!", name);
    return nullptr;
  }

  (*size) = myFile.size();
  
  char*buffer = (char*)ps_malloc((*size));
  if (buffer == nullptr){
    Logger::Error("sem ram amigo");
    return nullptr;
  }

  myFile.readBytes(buffer, (*size));
  
  myFile.close();

  return (unsigned char*)buffer;
} 


void Storage::FreeBuffer(uint16_t* ptr){
  heap_caps_free(ptr);
}

uint16_t* Storage::DecodeBMP(const char *name){
  size_t sz;
  unsigned char *buff = Storage::GetFileBinayContent(name, &sz);
  if (buff == nullptr){
    return nullptr;
  }

  int width = *(int*)&buff[18];
  int height = *(int*)&buff[22];

  if (width != 64 || height != 32){
    Logger::Error("Mismatched image size, expected %d %d but got %d %d", PANEL_WIDTH, PANEL_HEIGHT, width, height);
    heap_caps_free(buff);
    return nullptr;
  }
  uint16_t*pixels = (uint16_t*)ps_malloc( width * height * sizeof(uint16_t) );
  uint16_t *srcPtr = (uint16_t*)(buff + 54);
  int ptr = 0;
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++){
      pixels[ptr++] = *(srcPtr + j);
    }
    srcPtr += width; //Since 64 dont need padding, we're not calculating it here >:]
  }
  heap_caps_free(buff);
  return pixels;
}

PNG png;

void * myOpen(const char *filename, int32_t *size) {
  Storage::pngDecFile = SD.open(filename);
  *size = Storage::pngDecFile.size();
  return &Storage::pngDecFile;
}
void myClose(void *handle) {
  if (Storage::pngDecFile) Storage::pngDecFile.close();
}
int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length) {
  if (!Storage::pngDecFile) return 0;
  return Storage::pngDecFile.read(buffer, length);
}
int32_t mySeek(PNGFILE *handle, int32_t position) {
  if (!Storage::pngDecFile) return 0;
  return Storage::pngDecFile.seek(position);
}

// Function to draw pixels to the display
int PNGDraw(PNGDRAW *pDraw) {
  static uint16_t raw[128];
  png.getLineAsRGB565(pDraw, raw, PNG_RGB565_LITTLE_ENDIAN, 0xffffffff);
  memcpy(Storage::tmpBuffer + (pDraw->y *pDraw->iWidth), raw, pDraw->iWidth * sizeof(uint16_t));
  return 1;
}


uint16_t *Storage::DecodePNGForBuffer(const char *name, int& rcError){
  auto fp = Storage::getFile(name);
  if (!fp){
    rcError = 998;
    return nullptr;
  }
  fp.close();

  rcError = png.open(name, myOpen, myClose, myRead, mySeek, PNGDraw);
  if (rcError != PNG_SUCCESS) {
    return nullptr;
  }
  if (png.getWidth() != PANEL_WIDTH && png.getHeight() != PANEL_HEIGHT){
    Logger::Error("Mismatched image size, expected %d %d but got %d %d", PANEL_WIDTH, PANEL_HEIGHT, png.getWidth(), png.getHeight());
    png.close();
    rcError = 999;
    return nullptr;
  }
  Storage::tmpBuffer = (uint16_t*)ps_malloc(FILE_SIZE_BULK_SIZE );
  png.decode(NULL, 0);
  png.close();
  return Storage::tmpBuffer;
}

uint16_t *Storage::DecodePNG(const char *name, int &rcError, size_t &x, size_t &y){
  auto fp = Storage::getFile(name);
  if (!fp){
    rcError = 998;
    return nullptr;
  }
  fp.close();

  rcError = png.open(name, myOpen, myClose, myRead, mySeek, PNGDraw);
  if (rcError != PNG_SUCCESS) {
    return nullptr;
  }
  x = png.getWidth();
  y = png.getHeight();

  Storage::tmpBuffer = (uint16_t*)ps_malloc((x) * y * sizeof(uint16_t));
  png.decode(NULL, 0);
  png.close();
  return Storage::tmpBuffer;
}