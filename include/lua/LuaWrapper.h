#ifndef LUA_WRAPPER_H
#define LUA_WRAPPER_H

#include "Arduino.h"
#include <string>
#include <type_traits>
#include <vector>
#include <sstream>

#define LUA_USE_C89
#include "lua/lua.hpp"
typedef std::function<int(lua_State*)> LuaCFunctionLambda;

#define isthis(type,arg,FNC) if (type == arg) {vaav = std::string(#arg); if (FNC(L,i)){ std::stringstream S; S << FNC(L,i) << ";"; vaav = vaav +" "+ S.str(); }}

typedef struct PixelStruct{
    uint8_t x;
    uint8_t y;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

enum ReturnType{
    RETURN_NIL = -3,
    RETURN_ERROR = -2,
    RETURN_NOTFOUND = -1,
    RETURN_FALSE = 0,
    RETURN_TRUE = 1,
};


struct QuickDebug{

    static void DumpLua(lua_State *L,int start=-5,std::string msg=""){
        for (int i=start;i<5;i++){
            int type = lua_type(L,i);
            std::string vaav = "?";
            isthis(type,LUA_TNIL,lua_topointer);
            isthis(type,LUA_TNUMBER,lua_tonumber);
            isthis(type,LUA_TBOOLEAN,lua_tonumber);
            isthis(type,LUA_TSTRING,lua_tostring);
            isthis(type,LUA_TTABLE,lua_topointer);
            isthis(type,LUA_TFUNCTION,lua_topointer);
            isthis(type,LUA_TUSERDATA,lua_topointer);
            isthis(type,LUA_TTHREAD,lua_topointer);
            isthis(type,LUA_TLIGHTUSERDATA,lua_topointer);
            Serial.printf("%s[Dump: %d] %s\n",msg.c_str(),i,vaav.c_str());
        }
    };
};

template <int N> static int BaseLuaClosureHandler(lua_State *L){
    LuaCFunctionLambda **v = (LuaCFunctionLambda **)lua_touserdata(L, lua_upvalueindex(N));
    if (!v || !(*v)){
        luaL_error(L, "Could not call closure %d because of null or empty reference", N);
        return 1;
    }
    return (*(*v))(L);
}

template<typename T> inline T GenericLuaTonumber(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
    T n = 0;
    if (!lua_isnumber(L,stackPos)) {
        hasArgError = true;
        const char* function_name = lua_tostring(L, lua_upvalueindex(1));
        luaL_error(L, "Expected an numeric value on parameter %d of function %s, instead got %s", lua_gettop(L), function_name, lua_typename(L, lua_type(L, stackPos)));
        return n;
    }
    n = lua_tonumber(L,stackPos);
    if (pop)
        lua_pop(L,1);
    return n;
}


template<typename T> inline T GenericLuaString(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
    T n = "";
    if (!lua_isstring(L,stackPos)) {
        hasArgError = true;
        const char* function_name = lua_tostring(L, lua_upvalueindex(1));
        luaL_error(L, "Expected an string value on parameter %d of function %s", lua_gettop(L), function_name);
        return n;
    }
    n = lua_tostring(L,stackPos);
    if (pop)
        lua_pop(L,1);
    return n;
}

template<typename T> inline T GenericLuaBool(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
    T n;
    if (!lua_isboolean(L,stackPos)) {
        hasArgError = true;
        const char* function_name = lua_tostring(L, lua_upvalueindex(1));
        luaL_error(L, "Expected an string value on parameter %d of function %s", lua_gettop(L), function_name);
        return T(false);
    }
    n = lua_toboolean(L,stackPos);
    if (pop)
        lua_pop(L,1);
    return n;
}


template<typename T1> struct GenericLuaGetter{
     static inline  T1 Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return T1(GenericLuaTonumber<int>(hasArgError, L, stackPos, pop));
    };

    static T1 Empty;
};

/*
    void
*/

template<> struct GenericLuaGetter<uint16_t> {
    static inline uint16_t Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaTonumber<uint16_t>(hasArgError, L, stackPos, pop);
    }
};

template<> struct GenericLuaGetter<unsigned long> {
    static inline unsigned long Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaTonumber<unsigned long>(hasArgError, L, stackPos, pop);
    }
};

template<> struct GenericLuaGetter<char> {
    static inline char Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaTonumber<char>(hasArgError, L, stackPos, pop);
    }
};

template<> struct GenericLuaGetter<uint8_t> {
    static inline uint8_t Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaTonumber<uint8_t>(hasArgError, L, stackPos, pop);
    }
};

template<> struct GenericLuaGetter<uint32_t> {
    static inline uint32_t Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaTonumber<uint32_t>(hasArgError, L, stackPos, pop);
    }
};
template<> struct GenericLuaGetter<uint64_t> {
    static inline uint64_t Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaTonumber<uint64_t>(hasArgError, L, stackPos, pop);
    }
};
template<> struct GenericLuaGetter<int> {
    static inline int Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaTonumber<int>(hasArgError, L, stackPos, pop);
    }
};
template<> struct GenericLuaGetter<int16_t> {
    static inline int16_t Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaTonumber<int16_t>(hasArgError, L, stackPos, pop);
    }
};
template<> struct GenericLuaGetter<float> {
    static inline float Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaTonumber<float>(hasArgError, L, stackPos, pop);
    }
};

template<> struct GenericLuaGetter<PixelStruct> {
    static inline PixelStruct Call(bool &hasArgError, lua_State *L, int stackPos = -1, bool pop = true) {
        PixelStruct pixel = {0, 0, 0, 0, 0};

        if (!lua_istable(L, stackPos)) {
            hasArgError = true;
            const char* function_name = lua_tostring(L, lua_upvalueindex(1));
            luaL_error(L, "Expected a table value on parameter %d of function %s", lua_gettop(L), function_name);
            return pixel;
        }

        lua_getfield(L, stackPos, "x");
        pixel.x = static_cast<uint8_t>(luaL_optinteger(L, -1, 0));
        lua_pop(L, 1);

        lua_getfield(L, stackPos, "y");
        pixel.y = static_cast<uint8_t>(luaL_optinteger(L, -1, 0));
        lua_pop(L, 1);

        lua_getfield(L, stackPos, "r");
        pixel.r = static_cast<uint8_t>(luaL_optinteger(L, -1, 0));
        lua_pop(L, 1);

        lua_getfield(L, stackPos, "g");
        pixel.g = static_cast<uint8_t>(luaL_optinteger(L, -1, 0));
        lua_pop(L, 1);

        lua_getfield(L, stackPos, "b");
        pixel.b = static_cast<uint8_t>(luaL_optinteger(L, -1, 0));
        lua_pop(L, 1);

        if (pop) {
            lua_pop(L, 1); // Remove a tabela da pilha se necessário
        }

        return pixel;
    }
};

template<>
    struct GenericLuaGetter<std::string> {
     static inline std::string Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaString<std::string>(hasArgError, L, stackPos, pop);
    };
};

template<>
    struct GenericLuaGetter<String> {
     static inline String Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaString<String>(hasArgError, L, stackPos, pop);
    };
};



template<>
    struct GenericLuaGetter<const char*> {
     static inline const char* Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaString<const char*>(hasArgError, L, stackPos, pop);
    };
};


template<typename T> inline std::vector<T> GenericLuaVector(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
    std::vector<T> n;
    if (!lua_istable(L,stackPos)) {
        hasArgError = true;
        const char* function_name = lua_tostring(L, lua_upvalueindex(1));
        luaL_error(L, "Expected an uniform table value on parameter %d of function %s", lua_gettop(L), function_name);
        return n;
    }
    lua_pushnil(L);
    while (lua_next(L, 1)) {
        n.push_back(GenericLuaGetter<T>::Call(hasArgError, L, -1, true));
    }
    if (pop)
        lua_pop(L,1);
    return n;
}


template<>
    struct GenericLuaGetter<std::vector<int>> {
     static inline std::vector<int> Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaVector<int>(hasArgError, L, stackPos, pop);
    };
};

template<>
    struct GenericLuaGetter<std::vector<uint8_t>> {
     static inline std::vector<uint8_t> Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaVector<uint8_t>(hasArgError, L, stackPos, pop);
    };
};

template<>
    struct GenericLuaGetter<std::vector<PixelStruct>> {
     static inline std::vector<PixelStruct> Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaVector<PixelStruct>(hasArgError, L, stackPos, pop);
    };
};





template<typename T1> struct GenericLuaReturner{
    static inline void Ret(T1 vr,lua_State *L,bool forceTable = false){
        lua_pushnil(L);
    };
};

template<> struct GenericLuaReturner<char>{
     static inline void Ret(char vr,lua_State *L,bool forceTable = false){
        char aux[] = ".";
        aux[0] = vr;
        lua_pushstring(L,aux);
    };
};

template<> struct GenericLuaReturner<uint8_t>{
     static inline void Ret(uint8_t vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};

template<> struct GenericLuaReturner<unsigned long>{
    static inline void Ret(unsigned long vr,lua_State *L,bool forceTable = false){
        lua_pushnumber(L,vr);
   };
};

template<> struct GenericLuaReturner<uint32_t>{
     static inline void Ret(uint32_t vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};

template<> struct GenericLuaReturner<uint64_t>{
     static inline void Ret(uint64_t vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};

template<> struct GenericLuaReturner<int>{
     static inline void Ret(int vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};


template<> struct GenericLuaReturner<float>{
     static inline void Ret(float vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};


template<> struct GenericLuaReturner<int16_t>{
     static inline void Ret(int16_t vr,lua_State *L,bool forceTable = false){
        lua_pushnumber(L,vr);
    };
};

template<> struct GenericLuaReturner<void>{
     static inline void Ret(int vr,lua_State *L,bool forceTable = false){
        lua_pushnil(L);
    };
};


template<> struct GenericLuaReturner<uint16_t>{
     static inline void Ret(uint16_t vr,lua_State *L,bool forceTable = false){
         lua_pushnumber(L,vr);
    };
};

template<> struct GenericLuaReturner<bool>{
     static inline void Ret(bool vr,lua_State *L,bool forceTable = false){
        lua_pushboolean(L,vr);
    };
};

template<> struct GenericLuaReturner<std::string>{
     static inline void Ret(std::string vr,lua_State *L,bool forceTable = false){
        lua_pushstring(L,vr.c_str());
    };
};


template<> struct GenericLuaReturner<String>{
     static inline void Ret(String vr,lua_State *L,bool forceTable = false){
        lua_pushstring(L,vr.c_str());
    };
};

template<> struct GenericLuaReturner<const char *>{
     static inline void Ret(const char * vr,lua_State *L,bool forceTable = false){
        lua_pushstring(L,vr);
    };
};

typedef struct SizedArrayS{
    uint16_t *data;
    size_t size;
    void (*deleteAfterInsertion)(void *ptr) ;
} SizedArray;

template<> struct GenericLuaReturner<SizedArray*>{
    static inline void Ret(SizedArray *vr,lua_State *L,bool forceTable = false){
        if (vr == nullptr){
            lua_pushnil(L);
            return;
        }
        lua_newtable(L);
        for (int index=0; index <vr->size; index++) {
           lua_pushinteger(L, vr->data[index]);  
           lua_rawseti(L, -2, index+1 );
        }
        if (vr->deleteAfterInsertion != nullptr){
            vr->deleteAfterInsertion(vr->data);
        }
        delete []vr;
   };
};

template<> struct GenericLuaReturner<std::vector<uint16_t>>{
    static inline void Ret(std::vector<uint16_t> vr,lua_State *L,bool forceTable = false){
       lua_newtable(L);
       auto index = 1;
       for (const auto& value : vr) {
           lua_pushinteger(L, value);  
           lua_rawseti(L, -2, index++);
       }
   };
};

template<> struct GenericLuaReturner<std::vector<int>>{
     static inline void Ret(std::vector<int> vr,lua_State *L,bool forceTable = false){
        lua_newtable(L);
        auto index = 1;
        for (const auto& value : vr) {
            lua_pushinteger(L, value);  
            lua_rawseti(L, -2, index++);
        }
    };
};

template<> struct GenericLuaReturner<std::vector<uint8_t>>{
     static inline void Ret(std::vector<uint8_t> vr,lua_State *L,bool forceTable = false){
        lua_newtable(L);
        auto index = 1;
        for (const auto& value : vr) {
            lua_pushinteger(L, value);  
            lua_rawseti(L, -2, index++);
        }
    };
};

template<>
    struct GenericLuaGetter<bool> {
     static inline bool Call(bool &hasArgError, lua_State *L,int stackPos = -1,bool pop=true){
        return GenericLuaBool<bool>(hasArgError, L, stackPos, pop);
    }
};

template<typename ValueType> struct LuaTyper{
    static ValueType& GetTypeIfSame(ValueType aux,ValueType& karg){
        return karg;
    };
    template<typename K> static ValueType& GetTypeIfSame(ValueType aux,K& karg){
        ValueType *auxa;
        auxa = (ValueType *)&karg;
        return *auxa;
    };

};

template<class T> class LuaTypeConverterThing{
    public:
    static T Convert(T thing){
        return thing;
    };

    template<typename T2> static T& Convert(T2 &thing){
        static T st;
        T2 *aux = &thing;
        st = *((T*)(aux));
        return st;
    };
};


template<int N>
    struct readLuaValues {
    template<typename Tuple> static inline void Read(bool &hasArgError, Tuple& tuple,lua_State *L,int stackpos = -1,int offsetStack=0) {
        typedef typename std::tuple_element<N-1, Tuple>::type ValueType;
        ValueType v = GenericLuaGetter<ValueType>::Call(hasArgError, L,stackpos);
        if (hasArgError){
            return;
        }
        std::get<N-1>(tuple) = v;
        readLuaValues<N-1>::Read(hasArgError, tuple,L,stackpos,offsetStack);
    };

    template<typename Tuple,typename K,typename ... Opt> static inline void Read(bool &hasArgError, Tuple& tuple,lua_State *L,int stackpos,int offsetStack,K head, Opt ... tail) {
        typedef typename std::tuple_element<N-1, Tuple>::type ValueType;
        int argCountLua = lua_gettop(L)-offsetStack;
        if (N <= argCountLua){
            ValueType v = GenericLuaGetter<ValueType>::Call(hasArgError, L,stackpos);
            if (hasArgError){
                return;
            }
            std::get<N-1>(tuple) = v;
            readLuaValues<N-1>::Read(hasArgError, tuple,L,stackpos,offsetStack,head,tail...);
        }else{
            ValueType v = LuaTyper<ValueType>::GetTypeIfSame(ValueType(),head);
            std::get<N-1>(tuple) = v;
            readLuaValues<N-1>::Read(hasArgError, tuple ,L,stackpos,offsetStack, tail...);
        }
    };

     template<typename Tuple,typename K> static inline void Read(bool &hasArgError, Tuple& tuple,lua_State *L,int stackpos,int offsetStack,K headEnd) {
        typedef typename std::tuple_element<N-1, Tuple>::type ValueType;
        int argCountLua = lua_gettop(L)-offsetStack;
        if (N <= argCountLua){
            ValueType v = GenericLuaGetter<ValueType>::Call(hasArgError, L,stackpos);
            std::get<N-1>(tuple) = v;
            readLuaValues<N-1>::Read(hasArgError, tuple ,L,stackpos,offsetStack);
        }else{
            ValueType v = LuaTypeConverterThing<ValueType>::Convert(headEnd);
            std::get<N-1>(tuple) = v;
            readLuaValues<N-1>::Read(hasArgError, tuple,L,stackpos,offsetStack);
        }
    };
};

template<int N, typename Ret> struct expander {
    template<typename ArgStructure,typename Function, typename... Args>
        static inline Ret expand(const ArgStructure& a,lua_State *L,const Function& f, const Args&... args) {
            return expander<N-1,Ret>::expand(a,L,f, std::get<N-1>(a), args...);
    }

    template<typename ObjectName, typename ArgStructure,typename Function, typename... Args>
        static inline Ret expandClass(const ArgStructure& a,lua_State *L, ObjectName *obj, Function (ObjectName::*f), const Args&... args) {
            return expander<N-1,Ret>::expandClass(a, L, obj, f, std::get<N-1>(a), args...);
    }
};

template<typename Ret> struct finalCaller {
    template<typename F, typename... Args>
       static inline Ret functionCaller(const F& f,lua_State *L, const Args&... args) {
       Ret ret = f(args...);
       return (Ret)ret;
    };

    template<typename ObjectName, typename Fe, typename... Args>
       static inline Ret functionObjectCaller(Fe (ObjectName::*f),lua_State *L, ObjectName *obj, const Args&... args) {
       Ret ret = (obj->*f)(args...);
       return (Ret)ret;
    };
};

template<> struct finalCaller<void>{
    template<typename F, typename... Args>
       static inline void functionCaller(const F& f,lua_State *L, const Args&... args) {
       f(args...);
       return;
    };

    template<typename ObjectName, typename Fe, typename... Args>
        static inline void functionObjectCaller(Fe (ObjectName::*f),lua_State *L, ObjectName *obj, const Args&... args) {
        (obj->*f)(args...);
        return;
    };    
};

template<typename Ret> struct expander <0,Ret> {
    template<typename ArgStructure, typename Function, typename... Args>
        static inline Ret expand(const ArgStructure& a,lua_State *L,const Function& f, const Args&... args) {
            return finalCaller<Ret>::functionCaller(f,L,  args...);
    }

     template<typename ObjectName, typename ArgStructure, typename Function, typename... Args>
        static inline Ret expandClass(const ArgStructure& a,lua_State *L,  ObjectName *obj, Function (ObjectName::*f), const Args&... args) {
            return finalCaller<Ret>::functionObjectCaller(f,L, obj, args...);
    }
};

template<>
    struct readLuaValues<0> {
        template<typename Tuple> static inline void Read(bool &hasArgError,Tuple& tuple,lua_State *L,int stackpos=-1,int offsetStack=0) { };
        template<typename Tuple,typename K> static inline void Read(bool &hasArgError,Tuple& tuple,lua_State *L,int stackpos,int offsetStack, K k) { };
        template<typename Tuple,typename K,typename ... Opt> static inline void Read(bool &hasArgError,Tuple& tuple,lua_State *L,int stackpos,int offsetStack,K k,Opt ... aux) { };
};


// ------------- UTILITY---------------
template<int...> struct index_tuple{};
template<int I, typename IndexTuple, typename... Types> struct make_indexes_impl;
template<int I, int... Indexes, typename T, typename ... Types> struct make_indexes_impl<I, index_tuple<Indexes...>, T, Types...>{ typedef typename make_indexes_impl<I + 1, index_tuple<Indexes..., I>, Types...>::type type; };
template<int I, int... Indexes> struct make_indexes_impl<I, index_tuple<Indexes...> >{ typedef index_tuple<Indexes...> type; };
template<typename ... Types> struct make_indexes : make_indexes_impl<0, index_tuple<>, Types...>{};




template<int N,typename Tuple,typename ... Opt> static inline void TailOrganizer(bool &hasArgError, Tuple& tuple,lua_State *L,int stackpos,int offsetStack,Opt ... tail) {
    readLuaValues<N>::Read(hasArgError, tuple,L,stackpos,offsetStack,tail...);
}

template<int N,class Ret,class Tupler, class... Args, int... Indexes >
Ret apply_helper(bool &hasArgError,  Tupler &arglist,lua_State *L2,int k1,int k2, index_tuple< Indexes... >, std::tuple<Args...>&& tup)
{
    return TailOrganizer<N,Tupler,Args...>(hasArgError, arglist,L2,k1,k2, std::forward<Args>( std::get<Indexes>(tup)) ...); 
}
template<int N,class Ret,class Tupler, class ... Args>
Ret apply(bool &hasArgError, Tupler &arglist,lua_State *L2,int k1,int k2, std::tuple<Args...>&&  tup)
{
    return apply_helper<N,Ret,Tupler,Args...>(hasArgError, arglist, L2, k1, k2, typename make_indexes<Args...>::type(), tup);

}
template<int N,class Ret,class Tupler, class ... Args>
Ret apply(bool &hasArgError, Tupler &arglist,lua_State *L2,int k1,int k2, const std::tuple<Args...>&  tup)
{
    return apply_helper<N,Ret,Tupler,Args...>(hasArgError, arglist, L2, k1, k2, typename make_indexes<Args...>::type(), std::tuple<Args...>(tup));
}

//#define PRINT_LUA_CALLS


template <typename ... Types,typename ... Opt> void LambdaRegisterOpt(lua_State *L,std::string str, void func(Types ... args), Opt ... optionalArgs ){
    std::tuple<Opt...> tup(optionalArgs...);
    LuaCFunctionLambda f = [func,str,tup](lua_State *L2) -> int {
        int argNecessary = int(sizeof...(Types)) - int(sizeof...(Opt));
        int argMax = int(sizeof...(Types));
        int argCount = lua_gettop(L2);
        #ifdef PRINT_LUA_CALLS
        Serial.printf("Calling %s\n", str.c_str());
        #endif
        if (argCount > argMax){
            luaL_error(L2, "Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
            return 1;
        }

        if (argCount < argNecessary){
            luaL_error(L2, "Too few arguments on function %s. Expected %d-%d but got %d",str.c_str(),argNecessary, argMax, argCount);
            return 1;
        }
        std::tuple<Types ...> ArgumentList;
        bool hasArgError = false;
        apply<sizeof...(Types),void,std::tuple<Types ...>,Opt ...>(hasArgError,ArgumentList,L2,-1,0,tup);
        if (hasArgError){
            return 1;
        }
        expander<sizeof...(Types),void>::expand(ArgumentList,L2,func);
        return 0;
    };
    lua_pushstring(L, str.c_str());
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    lua_pushcclosure(L, BaseLuaClosureHandler<2>,2);
    lua_setglobal(L, str.c_str());
};

template <typename T1,typename ... Types,typename ... Opt> void LambdaRegisterOpt(lua_State *L,std::string str, T1 func(Types ... args), Opt ... optionalArgs ){
    std::tuple<Opt...> tup(optionalArgs...);
    LuaCFunctionLambda f = [func,str,tup](lua_State *L2) -> int {
        int argNecessary = int(sizeof...(Types)) - int(sizeof...(Opt));
        int argMax = int(sizeof...(Types));
        int argCount = lua_gettop(L2);
        #ifdef PRINT_LUA_CALLS
        Serial.printf("Calling %s\n", str.c_str());
        #endif
        if (argCount > argMax){
            luaL_error(L2, "Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
            return 1;
        }

        if (argCount < argNecessary){
            luaL_error(L2, "Too few arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
            return 1;
        }
        std::tuple<Types ...> ArgumentList;
        bool hasArgError = false;

        
        apply<sizeof...(Types),void,std::tuple<Types ...>,Opt ...>(hasArgError, ArgumentList,L2,-1,0,tup);
        if (hasArgError){
            return 1;
        }

        T1 rData = expander<sizeof...(Types),T1>::expand(ArgumentList,L2,func);
        GenericLuaReturner<T1>::Ret(rData,L2);
        return 1;
    };
    lua_pushstring(L, str.c_str());
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    lua_pushcclosure(L, BaseLuaClosureHandler<2>,2);
    lua_setglobal(L, str.c_str());
};

template<typename ObjectType, typename ... Types, typename ... Opt> void LambdaRegisterClassOpt(lua_State *L,std::string str, ObjectType *obj, void (ObjectType::*func)(Types ... args), Opt ... optionalArgs){
    std::tuple<Opt...> tup(optionalArgs...);
    LuaCFunctionLambda f = [obj,func,str,tup](lua_State *L2) -> int {
        int argNecessary = int(sizeof...(Types)) - int(sizeof...(Opt));
        int argMax = int(sizeof...(Types));
        int argCount = lua_gettop(L2);
        #ifdef PRINT_LUA_CALLS
        Serial.printf("Calling %s\n", str.c_str());
        #endif
        if (argCount > argMax){
            luaL_error(L2, "Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
            return 1;
        }

        if (argCount < argNecessary){
            luaL_error(L2, "Too few arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
            return 1;
        }
        std::tuple<Types ...> ArgumentList;
        bool hasArgError = false;
        apply<sizeof...(Types),void,std::tuple<Types ...>,Opt ...>(hasArgError, ArgumentList,L2,-1,0,tup);
        if (hasArgError){
            return 1;
        }
        expander<sizeof...(Types),void>::expandClass(ArgumentList,L2,obj,func);
        return 0;
    };
    lua_pushstring(L, str.c_str());
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    lua_pushcclosure(L, BaseLuaClosureHandler<2>,2);
    lua_setglobal(L, str.c_str());
};


template<typename T1, typename ObjectType, typename ... Types, typename ... Opt> void LambdaRegisterClassOpt(lua_State *L,std::string str, ObjectType *obj, T1 (ObjectType::*func)(Types ... args), Opt ... optionalArgs){
    std::tuple<Opt...> tup(optionalArgs...);
    LuaCFunctionLambda f = [obj,func,str,tup](lua_State *L2) -> int {
        int argNecessary = int(sizeof...(Types)) - int(sizeof...(Opt));
        int argMax = int(sizeof...(Types));
        int argCount = lua_gettop(L2);
        #ifdef PRINT_LUA_CALLS
        Serial.printf("Calling %s\n", str.c_str());
        #endif
        if (argCount > argMax){
            luaL_error(L2, "Too much arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
            return 1;
        }

        if (argCount < argNecessary){
            luaL_error(L2, "Too few arguments on function %s. Expected %d-%d but got %d\n",str.c_str(),argNecessary, argMax, argCount);
            return 1;
        }
        std::tuple<Types ...> ArgumentList;
        bool hasArgError = false;
        apply<sizeof...(Types),void,std::tuple<Types ...>,Opt ...>(hasArgError, ArgumentList,L2,-1,0,tup);
        if (hasArgError){
            return 1;
        }
        T1 rData = expander<sizeof...(Types),T1>::expandClass(ArgumentList,L2,obj,func);
        GenericLuaReturner<T1>::Ret(rData,L2);
        return 1;
    };
    lua_pushstring(L, str.c_str());
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    lua_pushcclosure(L, BaseLuaClosureHandler<2>,2);
    lua_setglobal(L, str.c_str());
};


template<typename ObjectType, typename ... Types> void LambdaRegisterClass(lua_State *L,std::string str, ObjectType *obj, void (ObjectType::*func)(Types ... args)){
    LuaCFunctionLambda f = [obj,func,str](lua_State *L2) -> int {
        int argCount = sizeof...(Types);
        int top = lua_gettop(L2);
        #ifdef PRINT_LUA_CALLS
        Serial.printf("Calling %s\n", str.c_str());
        #endif
        if (argCount > top){
            luaL_error(L2, "Too few arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,top);
            return 1;
        }
        if (argCount < top){
            luaL_error(L2, "Too much arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,top);
            return 1;
        }

        std::tuple<Types ...> ArgumentList;
        bool hasArgError = false;
        readLuaValues<sizeof...(Types)>::Read(hasArgError, ArgumentList,L2,-1,0);
        if (hasArgError){
            return 1;
        }
        expander<sizeof...(Types),void>::expandClass(ArgumentList,L2,obj,func);
        return 0;
    };
    lua_pushstring(L, str.c_str());
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    lua_pushcclosure(L, BaseLuaClosureHandler<2>,2);
    lua_setglobal(L, str.c_str());
};

template<typename ObjectType, typename T1,typename ... Types> void LambdaRegisterClass(lua_State *L,std::string str, ObjectType *obj, T1 (ObjectType::*func)(Types ... args)){
    LuaCFunctionLambda f = [obj,func,str](lua_State *L2) -> int {
        int argCount = sizeof...(Types);
        int top = lua_gettop(L2);
        #ifdef PRINT_LUA_CALLS
        Serial.printf("Calling %s\n", str.c_str());
        #endif
        if (argCount > top){
            luaL_error(L2, "Too few arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,top);
            return 1;
        }
        if (argCount < top){
            luaL_error(L2, "Too much arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,top);
            return 1;
        }

        std::tuple<Types ...> ArgumentList;
        bool hasArgError = false;
        readLuaValues<sizeof...(Types)>::Read(hasArgError, ArgumentList,L2,-1,0);
        if (hasArgError){
            return 1;
        }
        T1 rData = expander<sizeof...(Types),T1>::expandClass(ArgumentList,L2,obj,func);
        GenericLuaReturner<T1>::Ret(rData,L2);
        return 1;
    };
    lua_pushstring(L, str.c_str());
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    lua_pushcclosure(L, BaseLuaClosureHandler<2>,2);
    lua_setglobal(L, str.c_str());
};

template<typename T1,typename ... Types> void LambdaRegister(lua_State *L,std::string str, T1 func(Types ... args) ){
    LuaCFunctionLambda f = [func,str](lua_State *L2) -> int {
        int argCount = sizeof...(Types);
        int top = lua_gettop(L2);
        #ifdef PRINT_LUA_CALLS
        Serial.printf("Calling %s\n", str.c_str());
        #endif
        if (argCount > top){
            luaL_error(L2, "Too few arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,top);
            return 1;
        }
        if (argCount < top){
            luaL_error(L2, "Too much arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,top);
            return 1;
        }

        std::tuple<Types ...> ArgumentList;
        bool hasArgError = false;
        readLuaValues<sizeof...(Types)>::Read(hasArgError, ArgumentList,L2,-1,0);
        if (hasArgError){
            return 1;
        }
        T1 rData = expander<sizeof...(Types),T1>::expand(ArgumentList,L2,func);
        GenericLuaReturner<T1>::Ret(rData,L2);
        return 1;
    };
    lua_pushstring(L, str.c_str());
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    lua_pushcclosure(L, BaseLuaClosureHandler<2>,2);
    lua_setglobal(L, str.c_str());
};


template<typename ... Types> void LambdaRegister(lua_State *L,std::string str, void func(Types ... args) ){
    LuaCFunctionLambda f = [func,str](lua_State *L2) -> int {
        int argCount = sizeof...(Types);
        int top = lua_gettop(L2);
        #ifdef PRINT_LUA_CALLS
        Serial.printf("Calling %s\n", str.c_str());
        #endif
        if (argCount > top){
            luaL_error(L2, "Too few arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,top);
            return 1;
        }
        if (argCount < top){
            luaL_error(L2, "Too much arguments on function %s. Expected %d got %d\n",str.c_str(),argCount,top);
            return 1;
        }

        std::tuple<Types ...> ArgumentList;
        bool hasArgError = false;
        readLuaValues<sizeof...(Types)>::Read(hasArgError,ArgumentList,L2,-1,0);
        if (hasArgError){
            return 1;
        }
        expander<sizeof...(Types),void>::expand(ArgumentList,L2,func);
        return 0;
    };
    lua_pushstring(L, str.c_str());
    LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
    (*baseF) = new LuaCFunctionLambda(f);
    lua_pushcclosure(L, BaseLuaClosureHandler<2>,2);
    lua_setglobal(L, str.c_str());
};

/*
    Setting constants
*/

template<typename T>
typename std::enable_if<std::is_convertible<T, const char*>::value, void>::type
pushLuaValue(lua_State* L, const T& value) {
    lua_pushstring(L, value);
}

template<typename T>
typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, void>::type
pushLuaValue(lua_State* L, const T& value) {
    lua_pushinteger(L, value);
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
pushLuaValue(lua_State* L, const T& value) {
    lua_pushnumber(L, value);
}


class LuaWrapper {
  public:
    LuaWrapper();
    bool Lua_dostring(const char *script, int returns=0);
    bool Lua_dostring(const String *script, int returns=0){
        return Lua_dostring(script->c_str(), returns);
    }
    bool Lua_dostring(String &script, int returns=0){
        return Lua_dostring(&script, returns);
    }

    void FuncRegisterRaw(const char* name, int readSdFile(lua_State *L));

    void SetErrorCallback(void Func(const char*, lua_State *)){
        _errorCallback = Func;
    }

    template<typename T> void setConstant(const std::string& name, const T& value) {
        
        pushLuaValue(_state, value);
        lua_setglobal(_state, name.c_str());
    }


    template<typename T1,typename ... Types>void FuncRegister(std::string str, T1 func(Types ... args)){
      LambdaRegister(_state,str, func );
    }

    template<typename T1,typename ... Types, typename ... Opt>void FuncRegisterOptional(std::string str, T1 func(Types ... args), Opt ... optionalArgs){
      LambdaRegisterOpt(_state, str, func , optionalArgs... );
    }
    


    template<typename ObjectName, typename T1,typename ... Types, typename ... Opt>void FuncRegisterFromObjectOpt(std::string str, ObjectName *obj, T1 (ObjectName::*func)(Types ... args), Opt ... optionalArgs){
      LambdaRegisterClassOpt(_state,str, obj, func, optionalArgs... );
    }

    template<typename ObjectName, typename T1,typename ... Types>void FuncRegisterFromObject(std::string str, ObjectName *obj, T1 (ObjectName::*func)(Types ... args)){
      LambdaRegisterClass(_state,str, obj, func);
    }
    


    template<typename... Args>
    bool callLuaFunction(const std::string& functionName, Args&&... args) {
        lua_getglobal(_state, functionName.c_str());

        if (!lua_isfunction(_state, -1) || lua_isnil(_state, -1)) {
            lua_pop(_state, 1);
            if (_errorCallback != nullptr){
                _errorCallback("Called function is nil or not a function", _state);
            }
            return false;
        }

        (void)std::initializer_list<int>{(pushParam(std::forward<Args>(args)), 0)...};

        int numParams = sizeof...(args);

        if (lua_pcall(_state, numParams, 0, 0) != 0) {
            const char* errorMessage = lua_tostring(_state, -1);
            lua_pop(_state, 1);
            if (_errorCallback != nullptr){
                _errorCallback(errorMessage, _state);
            }
            return false;
        }
        
        return true;
    };

    void printStack(){
        QuickDebug::DumpLua(_state);
    }
    lua_State *GetState(){
        return _state;
    }

  private:

    void (*_errorCallback)(const char*,lua_State *L);
    lua_State *_state;

    void pushParam(std::string parameter){
      lua_pushstring(_state, parameter.c_str());
    }
    void pushParam(int parameter){
      lua_pushnumber(_state, parameter);
    }

    void pushParam(float parameter) {
        lua_pushnumber(_state, parameter);
    }

    void pushParam(int64_t parameter) {
        lua_pushinteger(_state, parameter);
    }

    void pushParam(double parameter) {
        lua_pushnumber(_state, parameter);
    }

    void pushParam(char* parameter) {
        lua_pushstring(_state, parameter);
    }

    void pushParam(const char* parameter) {
        lua_pushstring(_state, parameter);
    }

    void pushParam(char parameter) {
        lua_pushinteger(_state, static_cast<int>(parameter));
    }

    void pushParam(uint8_t parameter) {
        lua_pushinteger(_state, parameter);
    }

    void pushParam(uint32_t parameter) {
        lua_pushinteger(_state, parameter);
    }

    template<typename T>
    void pushParam(const std::vector<T>& parameters) {
        lua_createtable(_state, static_cast<int>(parameters.size()), 0);
        
        for (size_t i = 0; i < parameters.size(); ++i) {
            pushParam(parameters[i]);                  
            lua_rawseti(_state, -2, static_cast<int>(i + 1)); 
        }
    }
};



#endif
