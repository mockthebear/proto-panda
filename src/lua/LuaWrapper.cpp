#include "lua/LuaWrapper.h"
#include "esp32-hal.h"
#include <SD.h>

void CreateLuaClosure(lua_State *L, const std::function<int(lua_State*)>& f){
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    void* mem = ps_malloc(sizeof(LuaCFunctionLambda));
    if (!mem) {
        luaL_error(L, "Failed to allocate PSRAM for Lua function");
        return;
    }
    (*baseF) = new (mem) LuaCFunctionLambda(f);
}
//Make sure that any lua scripts use the psram instead of the sram
static void *psram_lua_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)ud;  (void)osize; 
  if (nsize == 0) { 
    heap_caps_free(ptr);
    return NULL;
  }else{
    return ps_realloc(ptr, nsize);
  }
}

int aux_esp_fclose(EspFile *fp){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  espf->close();
  delete espf;
  return 0;
}

int aux_esp_feof(EspFile *fp){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  return espf->available() == 0 ? 1 : 0;
}

int aux_esp_getc(EspFile *fp){
  //Pop the ungetc
  if (fp->lastUngetC != EOF){
    int aux = fp->lastUngetC;
    fp->lastUngetC = EOF;
    return aux;
  }
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  int aux = espf->read();
  if (aux == EOF){
    fp->hasError = 1;
  }
  return aux;
}

int aux_esp_ungetc(int n,EspFile *fp){
  fp->lastUngetC = n;
  if (n == EOF){
    return EOF;
  }
  return 0;
}

int aux_esp_fflush(EspFile *fp){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  espf->flush();
  return 0;
}

int aux_esp_ferror(EspFile *fp){
  return fp->hasError;
}

void aux_esp_clearerr(EspFile *fp){
  fp->hasError = 0;
}

long aux_esp_ftell(EspFile *fp){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  return espf->position();
}

int	aux_esp_fseek(EspFile *fp, long a , int b){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  SeekMode mode = SeekSet;
  if (mode == SEEK_SET){
    mode = SeekSet;
  }else if (mode == SEEK_CUR){
    mode = SeekCur;
  }else if (mode == SEEK_END){
    mode = SeekEnd;
  }
  return espf->seek(a, mode);
}

size_t aux_esp_fread(char * buffer, size_t _size, size_t _n, struct EspFile *fp){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  return espf->readBytes(buffer, _size*_n);
}

size_t aux_esp_fwrite(const void * buffer, size_t _size, size_t _n, struct EspFile *fp){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  return espf->write((const uint8_t*)buffer, _size*_n);
}
int aux_esp_fprintf(EspFile * fp, const char *buff, LUAI_UACNUMBER num){
  File *espf = (File*)fp->actualFilePtr;
  if (espf == nullptr){
    return EOF;
  }
  return espf->printf(buff, num);
}

EspFile *custom_fopen(lua_State *L, const char* filename, const char *mode){
  if (filename == NULL){
    return NULL;
  }
  File *myFile = new File(SD.open(filename, mode));
  if (*myFile){
    EspFile *fp = new EspFile;
    fp->lastUngetC = -1;
    fp->hasError = 0;
    fp->L = L;
    fp->actualFilePtr = (void*)myFile;
    fp->esp_fclose = aux_esp_fclose;
    fp->esp_feof = aux_esp_feof;
    fp->esp_getc = aux_esp_getc;
    fp->esp_ungetc = aux_esp_ungetc;
    fp->esp_ferror = aux_esp_ferror;
    fp->esp_clearerr = aux_esp_clearerr;
    fp->esp_ftell = aux_esp_ftell;
    fp->esp_fseek = aux_esp_fseek;
    fp->esp_fflush = aux_esp_fflush;
    fp->esp_fread = aux_esp_fread;
    fp->esp_fwrite = aux_esp_fwrite;
    fp->esp_fprintf = aux_esp_fprintf;
    return fp;
  }
  return NULL;
}

void custom_serial(const char* msg){
  Serial.print(msg);
}

LuaWrapper::LuaWrapper() {
  _errorCallback = nullptr;
  _state = lua_newstate(psram_lua_alloc, NULL);
  lua_setallocf(_state, psram_lua_alloc, NULL);

  lua_setfopenf_esp(_state, custom_fopen); 
  lua_setserial_esp(_state, custom_serial); 

  luaL_openlibs(_state);
  luaopen_math(_state);
  luaopen_table(_state);

  const char* lua_require_code = R"(
_G.require = nil
function require(packageName)
    if not _G.package then 
        _G.package = {}
    end
    if not _G.package[packageName] then 
        local name = packageName:gsub("%.", "/")
        local notFound = {}
        for path in string.gmatch(package.path..';', "(.-);") do 
            local dir = path:gsub("%?", name)
            local success, data = pcall(dofile, dir)
            if not success then 
                if data:match("^cannot open") then
                    notFound[#notFound+1] = dir
                else 
                    error(data)
                end
            else 
                _G.package[packageName] = data 
                _G[packageName] = data
                return data
            end
        end
        local str = "module '"..packageName.."' not found:\nno field package.preload['"..packageName.."']\n"
        for i,b in pairs(notFound) do  
            str = str ..'\tno file \''..b..'\'\n'
        end
        error(str)
    else 
        return _G.package[packageName]
    end
end
    )";

  if (luaL_dostring(_state, lua_require_code) != 0) {
    lua_pop(_state, 1); 
  }  
}


void LuaWrapper::FuncRegisterRaw(const char* name, int functionPtr(lua_State *L)){
  lua_register(_state, name, functionPtr);
}

bool LuaWrapper::Lua_dostring(const char *script, int returns) {
  int error;
    
  error = luaL_loadstring(_state, script);
  if (error) {
      if (_errorCallback != nullptr){
        _errorCallback(lua_tostring(_state, -1), _state);
      }
      lua_pop(_state, 1);
      return false;
  }

  if (lua_pcall(_state, 0, returns, 0)) {
    if (_errorCallback != nullptr){
        _errorCallback(lua_tostring(_state, -1), _state);
      }
    lua_settop(_state, 0);
    return false;
  }

  return true;
}
