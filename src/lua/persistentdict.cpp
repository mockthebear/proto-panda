#include "lua/persistentdict.hpp"
#include <SD.h>
#include <stdio.h>
#include <string.h>
#include "tools/logger.hpp"

void PersistentDictionary::Format(){
    store.clear();
    SD.remove("/dict.bin");
}

void PersistentDictionary::Save() {
    File outfile = SD.open("/cache/dict.bin", FILE_WRITE);
    if (!outfile) {
        Logger::Error("Failed to open file for writing!");
        return;
    }

    Logger::Info("Saving dictionary...");
    for (const auto& kv : store) {
        size_t key_size = kv.first.size();
        outfile.write((uint8_t*)&key_size, sizeof(key_size));
        outfile.write((uint8_t*)kv.first.c_str(), key_size);

        size_t val_size = kv.second.size();
        outfile.write((uint8_t*)&val_size, sizeof(val_size));
        outfile.write((uint8_t*)kv.second.c_str(), val_size);

    }
    outfile.close();
    Logger::Info("Dictionary saved successfully.");
}

void PersistentDictionary::Load() {
    Logger::Info("Loading dictionary...");

    File infile = SD.open("/cache/dict.bin", FILE_READ);
    if (!infile) {
        set("clear", "1");
        Save();
        Logger::Info("No dictionary file found.");
        return;
    }

    while (infile.available()) {
        size_t key_size;
        if (infile.read((uint8_t*)&key_size, sizeof(key_size)) != sizeof(key_size)) {
            Logger::Error("Failed to read key size!");
            break;
        }

        PSRAMString key_content(key_size, '\0');
        if (infile.read((uint8_t*)key_content.data(), key_size) != key_size) {
            Logger::Error("Failed to read key content!");
            break;
        }

        size_t val_size;
        if (infile.read((uint8_t*)&val_size, sizeof(val_size)) != sizeof(val_size)) {
            Logger::Error("Failed to read value size!");
            break;
        }

        PSRAMString value_content(val_size, '\0');
        if (infile.read((uint8_t*)value_content.data(), val_size) != val_size) {
            Logger::Error("Failed to read value content!");
            break;
        }

        if (store.find(key_content) == store.end()) {
            store[key_content] = value_content;
        }
    }

    infile.close();
    Logger::Info("Dictionary loaded successfully.");
}


void PersistentDictionary::set(std::string key, const char* val){
    
    PSRAMString psramKey(key.c_str(), PSRAMAllocator<char>());
    PSRAMString psramValue(val, PSRAMAllocator<char>());
    store[psramKey] = psramValue;
}

const char* PersistentDictionary::get(std::string key){
    PSRAMString psramKey(key.c_str(), PSRAMAllocator<char>());
    if (store.find(psramKey) != store.end()) {
        return store[psramKey].c_str();
    } else {
        return ""; // or handle the case where the key doesn't exist
    }
}

void PersistentDictionary::del(std::string key){
    store.erase(key.c_str());
}
