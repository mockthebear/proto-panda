#include <map>
#include <string>
#include <memory>
#include "esp32-hal.h"
#include "tools/logger.hpp"
#include "tools/psrammap.hpp"


class PersistentDictionary {
public:
    PersistentDictionary(){};

    void Save();
    void Load();
    void Format();

    void set(std::string key, const char* val);

    const char* get(std::string key);

    void del(std::string key);

private:
    PSRAMMap store;    
};