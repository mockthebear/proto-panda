#include "tools/compression.hpp"
#include <Arduino.h>

int Compression::Compress(uint16_t *m_compressionBuffer, uint16_t* originalData, int dataLen){
    /* 
        This algorythm is based on LZ4...
        But not really.
        It just check how much a byte repeats. If the byte in question does not repeat alot and we have a section with alot of entropy
        then we classify as a "raw section". That avoid adding alot of 1x color Y, 1x color Z, 1x color A... That save up alot of bytes tho
    */
    uint16_t current = originalData[0];
    uint16_t repetitions = 1;
    uint8_t *toWriteData = (uint8_t*)m_compressionBuffer;
    int bytesUsed = 0;
    bool skipLastInsert = false;
    int rawCount = 0;
    int readtotal = dataLen/sizeof(uint16_t);
    int pixelId = 0;
    for (int i=1;i<readtotal ;i++){
        if (originalData[i] == current && repetitions < 250){
            repetitions++;
        }else{
            if (repetitions == 1 && rawCount < 250){
                if (i+1 <= readtotal){
                    skipLastInsert = true;
                    uint16_t prevColor = originalData[i];
                    int diffs = 1;
                    int threshhold = 2;
                    for (int a=i;a<readtotal;a++){
                        diffs++;
                        if (current != originalData[a] && prevColor != originalData[a]){
                            threshhold=3;
                        }else{
                            threshhold--;
                            if (threshhold <= 0){
                                break;
                            }
                        }
                        if (diffs > 250){
                            break;
                        }
                    }
                    toWriteData[bytesUsed++] = 253;
                    toWriteData[bytesUsed++] = diffs+1;
                    toWriteData[bytesUsed++] = current & 0xFF;       
                    toWriteData[bytesUsed++] = (current >> 8) & 0xFF; 
                    int ee = 1;
                    for (int a=0;a<diffs;a++){
                        current = originalData[i+a];
                        toWriteData[bytesUsed++] = current & 0xFF;       
                        toWriteData[bytesUsed++] = (current >> 8) & 0xFF; 
                        ee++;
                    }
                    pixelId += diffs+1;
                    i += diffs;                    
                }else{
                    break;
                }
            }else{
                toWriteData[bytesUsed++] = repetitions;
                toWriteData[bytesUsed++] = current & 0xFF;       
                toWriteData[bytesUsed++] = (current >> 8) & 0xFF; 
                pixelId += repetitions;
                
            }
            skipLastInsert = false;
            repetitions = 1;
            current = originalData[i];
        }
    }
    if (!skipLastInsert){
        toWriteData[bytesUsed++] = repetitions;
        toWriteData[bytesUsed++] = current & 0xFF;       
        toWriteData[bytesUsed++] = (current >> 8) & 0xFF;
        pixelId += repetitions;
    }

    return bytesUsed;
}


int Compression::Decompress(uint16_t *m_decompressionBuffer, int decompressedSize, uint16_t* originalData, int compressedSize){
    int compressionReadPos = 0;
    int decompressedReadPos = 0;
    uint8_t *readBuffer = (uint8_t*)originalData;
    do{
        int lenght = readBuffer[compressionReadPos++];
        int iter = 1;
        if (lenght == 253){
            if (compressionReadPos >= compressedSize){
                return 1;
            }
            iter = readBuffer[compressionReadPos++];
            lenght = 1;
        }
        while(iter > 0){
            uint16_t color = readBuffer[compressionReadPos++]; 
            if (compressionReadPos >= compressedSize){
                return 1;
            }
            color |= (readBuffer[compressionReadPos++] << 8); 
            for (int iddx=0;iddx<lenght;iddx++){
                m_decompressionBuffer[decompressedReadPos++] = color;
                if (compressionReadPos >= decompressedSize){
                    return 0;
                }   
            }   
            if (compressionReadPos >= compressedSize){
                return 1;
            }      
        }
    } while(compressionReadPos < decompressedSize);
    return 0;
}