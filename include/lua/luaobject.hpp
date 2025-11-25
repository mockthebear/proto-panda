#pragma once 

#include "lua/LuaWrapper.h"
#include "bluetooth/characteristicshandler.hpp"
#include "bluetooth/servicehandler.hpp"
#include <Arduino.h>

class Batata{
    public:
    Batata(){count = 0;};
    ~Batata(){
        Serial.printf("THE BATATA %d IS DEAD MAN\n", (int)this);
    };
    int SumBatata(Batata *bat){
        return count + bat->Get();
    };
    Batata *CloneBatata(){
        Serial.printf("CLONE, I AM %d\n", (int)this);
        Batata *aux = new Batata();
        Serial.printf("THE NEW IS %d\n", (int)aux);
        aux->Set(Get());
        return aux;
    }
    uint32_t Get(){return count;};
    void Set(uint32_t c){ count = c;};
    uint32_t Sum(uint32_t a, uint32_t b){ count += a+b; return count;};
    uint32_t count;
};




class LuaCaller{
    public:
        template <typename T> static T* GetSelf(lua_State * L, int pos = -1){
            lua_getfield(L, pos, "__self");
            T** data = (T**)lua_touserdata(L, -1);
            if (!data){
                return nullptr;
            }
            return (*data);
        }

        template <int N> static int BaseEmpty(lua_State *L){
            if (!L){
                return 0;
            }
            LuaCFunctionLambda **v = (LuaCFunctionLambda **)lua_touserdata(L, lua_upvalueindex(N));
            if (!v || !(*v)){
                lua_pushstring(L,"[LUA]could not call closure %d because null reference");
                lua_error (L);
                return 0;
            }
            (*(*v))(L);
            return 1;
        }

        
        template <typename T> static int GC(lua_State *L){
            T** part = (T**)lua_touserdata(L, -1);
            if (part && *part){
                delete (*part);
                (*part) = nullptr;
                return 1;
            }
            return 0;

        }

        template <int N> static int Base(lua_State *L){
            LuaCFunctionLambda **v = (LuaCFunctionLambda **)lua_touserdata(L, lua_upvalueindex(N));
            if (!v || !(*v)){
                lua_pushstring(L,"[LUA]could not call closure %d because null reference");
                lua_error (L);
                return 0;
            }

            if (lua_gettop(L) < 1 || !lua_istable(L, 1)) {
                lua_pushstring(L, "[LUA] Method called with dot instead of colon. Use 'obj:method()' not 'obj.method()'");
                lua_error(L);
                return 0;
            }

            (*(*v))(L);
            return 1;
        }
};



template<typename Ret,typename Ctype, typename F, typename... Args> struct internal_caller{
       static Ret functionCallerClass(const F& f,lua_State *L, const Args&... args) {
       Ctype* part = LuaCaller::GetSelf<Ctype>(L);
       if (!part){
            return Ret();
       }
       Ret ret = ((part)->*f)(args...);
       return (Ret)ret;
    };
};
/*
    Its void member
*/
template<typename Ctype, typename F, typename... Args> struct internal_caller<void,Ctype, F, Args...>{
       static void functionCallerClass(const F& f,lua_State *L, const Args&... args) {
       Ctype* part = LuaCaller::GetSelf<Ctype>(L);
       if (!part){
            lua_pushstring(L,"[LUA]Could not call function because self reference is a nullpointer.");
            lua_error (L);
            return;
       }
       ( (part)->*f)(args...);
       return;
    };
};

template<int N,typename Ctype, typename Ret> struct expanderClass {
    template<typename ArgStructure,typename Function, typename... Args>
        static Ret expand(const ArgStructure& a,lua_State *L,const Function& f, const Args&... args) {
            return expanderClass<N-1,Ctype,Ret>::expand(a,L,f, std::get<N-1>(a), args...);
    }
};

template<typename Ctype,typename Ret> struct expanderClass <0,Ctype,Ret> {
    template<typename ArgStructure, typename Function, typename... Args>
        static Ret expand(const ArgStructure& a,lua_State *L,const Function& f, const Args&... args) {
            return internal_caller<Ret,Ctype,Function,Args...>::functionCallerClass(f,L,  args...);
    }
};


template<typename T1,typename ClassObj,typename ... Types> struct internal_register{

    template <typename ... Opt> static void LambdaRegisterStackOpt(lua_State *L,std::string str,int stackPos,T1 (ClassObj::*func)(Types ... args),Opt ... optionalArgs ){
        std::tuple<Opt...> tup(optionalArgs...);
        LuaCFunctionLambda f = [func,str,tup](lua_State *L2) -> int {
            int argNecessary = int(sizeof...(Types)) - int(sizeof...(Opt));
            int argMax = int(sizeof...(Types));
            int argCount = lua_gettop(L2)-1; //Ignore the first one that is the table!

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
            apply<sizeof...(Types),void,std::tuple<Types ...>,Opt ...>(hasArgError, ArgumentList,L2,-1,1,tup);
            if (hasArgError){
                luaL_error(L2, "Wrong arguments");
                return 1;
            }

            T1 rData = expanderClass<sizeof...(Types),ClassObj,T1>::expand(ArgumentList,L2,func);
            GenericLuaReturner<T1>::Ret(rData,L2);
            return 1;
        };
        LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
        (*baseF) = new LuaCFunctionLambda(f);
        lua_pushcclosure(L, LuaCaller::Base<1>,1);
        lua_setfield(L, -2, str.c_str());
        lua_pop(L, 1);

    };
};

template<typename ClassObj,typename ... Types> struct internal_register<void,ClassObj,Types...>{

    template <typename ... Opt> static void LambdaRegisterStackOpt(lua_State *L,std::string str,int stackPos,void (ClassObj::*func)(Types ... args),Opt ... optionalArgs ){
        std::tuple<Opt...> tup(optionalArgs...);
        LuaCFunctionLambda f = [func,str,tup](lua_State *L2) -> int {
            int argNecessary = int(sizeof...(Types)) - int(sizeof...(Opt));
            int argMax = int(sizeof...(Types));
            int argCount = lua_gettop(L2)-1; //-1 because of the table thats the root
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
            apply<sizeof...(Types),void,std::tuple<Types ...>,Opt ...>(hasArgError, ArgumentList,L2,-1,1,tup);
            //readLuaValues<sizeof...(Types)>::Read(ArgumentList,L2,-1,1,optionalArgs...);
            if (hasArgError){
                luaL_error(L2, "Wrong arguments");
                return 1;
            }
            expanderClass<sizeof...(Types),ClassObj,void>::expand(ArgumentList,L2,func);
            GenericLuaReturner<void>::Ret(0,L2);

            return 1;
        };
        LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
        (*baseF) = new LuaCFunctionLambda(f);
        lua_pushcclosure(L, LuaCaller::Base<1>,1);
        lua_setfield(L, -2,  str.c_str());

    };
};


template<typename T1,typename ObjT> struct FieldToLuaHandler{
    /**
        Modify field
    */
    static int Newindex(lua_State *L){
        bool hasErr = false;
        ObjT         data = GenericLuaGetter<ObjT>::Call(hasErr, L,-1);
        if (hasErr){
            return 1;
        }
        std::string field = lua_tostring(L,-1);
        lua_pop(L, 1);        
        T1 *self = LuaCaller::GetSelf<T1>(L, -1);
        if (!self){
            return 0;
        }
        std::map<std::string,ObjT T1::*> &fieldData = FieldToLuaHandler<T1,ObjT>::getAddr();
        if (fieldData[field]){
            ObjT T1::* fieldptr = fieldData[field];
            self->*fieldptr = data;
        }
        return 0;
    }
    /**
        Request field
    */
    static int Index(lua_State *L){
        
        std::string field = lua_tostring(L,-1);
        T1 *self = LuaCaller::GetSelf<T1>(L, -2);
        if (!self){
			return 0;
        }
        std::map<std::string,ObjT T1::*> &fieldData = FieldToLuaHandler<T1,ObjT>::getAddr();
        if (fieldData[field]){
            ObjT T1::* fieldptr = fieldData[field];
            GenericLuaReturner<ObjT>::Ret(self->*fieldptr,L);
            return 1;
        }
        return 0;
    };
   
    static bool HasField(std::string fieldName){
        std::map<std::string,ObjT T1::*> &fieldData = FieldToLuaHandler<T1,ObjT>::getAddr();
        return fieldData[fieldName] != nullptr;
    };
    
    static std::map<std::string,ObjT T1::*> &getAddr(){
        static std::map<std::string,ObjT T1::*> addr;
        return addr;
    };
};
/*
    Used to call the proper newindex/index or so of the given and correct type
*/
template<typename T1> struct IndexerHelper{
    static int Newindex(lua_State *L){
        std::string field = lua_tostring(L,-2);
        if (FieldToLuaHandler<T1,int>::HasField(field)){
            return FieldToLuaHandler<T1,int>::Newindex(L);
        }else if (FieldToLuaHandler<T1,float>::HasField(field)){
            return FieldToLuaHandler<T1,float>::Newindex(L);
        }else if (FieldToLuaHandler<T1,std::string>::HasField(field)){
            return FieldToLuaHandler<T1,std::string>::Newindex(L);
        }else if (FieldToLuaHandler<T1,bool>::HasField(field)){
            return FieldToLuaHandler<T1,bool>::Newindex(L);
        }else if (FieldToLuaHandler<T1,uint32_t>::HasField(field)){
            return FieldToLuaHandler<T1,uint32_t>::Newindex(L);
        }else if (FieldToLuaHandler<T1,uint16_t>::HasField(field)){
            return FieldToLuaHandler<T1,uint16_t>::Newindex(L);
        }else if (FieldToLuaHandler<T1,uint8_t>::HasField(field)){
            return FieldToLuaHandler<T1,uint8_t>::Newindex(L);
        }else if (FieldToLuaHandler<T1,size_t>::HasField(field)){
            return FieldToLuaHandler<T1,size_t>::Newindex(L);
        }
        return 0;
    };
    static int Index(lua_State *L){
        std::string field = lua_tostring(L,-1);
        if (FieldToLuaHandler<T1,int>::HasField(field)){
            return FieldToLuaHandler<T1,int>::Index(L);
        }else if (FieldToLuaHandler<T1,float>::HasField(field)){
            return FieldToLuaHandler<T1,float>::Index(L);
        }else if (FieldToLuaHandler<T1,std::string>::HasField(field)){
            return FieldToLuaHandler<T1,std::string>::Index(L);
        }else if (FieldToLuaHandler<T1,bool>::HasField(field)){
            return FieldToLuaHandler<T1,bool>::Index(L);
        }else if (FieldToLuaHandler<T1,uint32_t>::HasField(field)){
            return FieldToLuaHandler<T1,uint32_t>::Index(L);
        }else if (FieldToLuaHandler<T1,uint8_t>::HasField(field)){
            return FieldToLuaHandler<T1,uint8_t>::Index(L);
        }else if (FieldToLuaHandler<T1,uint16_t>::HasField(field)){
            return FieldToLuaHandler<T1,uint16_t>::Index(L);
        }else if (FieldToLuaHandler<T1,uint16_t>::HasField(field)){
            return FieldToLuaHandler<T1,uint16_t>::Index(L);
        }else if (FieldToLuaHandler<T1,size_t>::HasField(field)){
            return FieldToLuaHandler<T1,size_t>::Index(L);
        }
        return 0;
    };
};

template<typename T1> struct ClassRegister{
    static void RegisterObject(lua_State *L, std::string name, T1 *obj, bool ForceMeatable=true){
        if (!obj){
            lua_pushstring(L, "[LUA] on RegisterObject, null reference.");
            lua_error(L);
            return;
        }
        //The object!
        //local obj = {}
        lua_newtable(L);
        //Register ID
        //obj.id = something
        lua_pushstring(L, "id");
        lua_pushnumber(L, (uint64_t)obj);
        lua_settable(L, -3);
        //Type as string
        //obj.type = 'name'
        lua_pushstring(L, "type");
        lua_pushstring(L, name.c_str());
        lua_settable(L, -3);
        //Now create a table
        //local data = {}
        lua_pushstring(L, "data"); //Here we're saying: obj['data'] = <what follows>
        lua_newtable(L);
        //data.__self = userdata
        lua_pushstring(L, "__self");
        T1 **usr = static_cast<T1**>(lua_newuserdata(L, sizeof(T1*)));
        *usr = obj;
        lua_settable(L, -3);
        
        //local meta = {}
        lua_newtable(L);
        //meta.__index = <func>
        lua_pushstring(L, "__index");
        lua_pushcfunction(L, IndexerHelper<T1>::Index);
        lua_settable(L, -3);
        //meta.__newindex = <func>
        lua_pushstring(L, "__newindex");
        lua_pushcfunction(L, IndexerHelper<T1>::Newindex);
        lua_settable(L, -3);
        //setmetatable(data, meta)
        lua_setmetatable(L, -2); 
        //obj.data = data //basically the completion of lua_pushstring(L, "data");
        lua_settable(L, -3);
        //obj.__self = userdata
        lua_pushstring(L, "__self");
        T1 **usr2 = static_cast<T1**>(lua_newuserdata(L, sizeof(T1*)));
        *usr2 = obj;
        lua_settable(L, -3);
        //We have a completed object 'obj'
        //setmetatable(obj, metatable_name)
        luaL_getmetatable(L, name.c_str());
        lua_setmetatable(L, -2);     

    };

     static void RegisterClassType(lua_State *L,std::string name,
		 std::function<T1*(lua_State*)> makerF = std::function<T1*(lua_State*)>(),
                                      LuaCFunctionLambda *gc_func = nullptr){
        //local class = {}
        lua_newtable(L);
        // _G[name] = class
        lua_pushvalue(L, -1);
        lua_setglobal(L, name.c_str());
        //local methods = {}
        lua_newtable(L);
        //Creating the method so when wer call name() it executes
        lua_pushstring(L, "__call");
        static LuaCFunctionLambda Flambb = [name,makerF](lua_State* Ls) -> int{
			T1 *obj = makerF(Ls);
			if (!obj){
                return 1;
			}
			RegisterObject(Ls,name,obj,false);
            return 1;
        };
        //When calling methods()
        LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
        (*baseF) = &Flambb;

        //methods.
        lua_pushcclosure(L, LuaCaller::BaseEmpty<1>,1);
        lua_settable(L, -3);
        //When executing name() it will call a c closure BaseEmpty with 1 parameter. The parameter is the userdata for the lambda function address. Then BaseEmpty will deal with it lol
        //setmetatable(class, methods)
        lua_setmetatable(L, -2);
        //Technically we have it.
        //if we do lua_pop(L, 1); we leave without 'returning' anything.
        //1 on the stack is 'class'
        //-1 in stack is 'class'

        //Now we create a global metatable named 'name'
        luaL_newmetatable(L, name.c_str()); //metatable is in -1 and class is -2

        lua_pushstring(L, "__metatable"); //Now this is -1 '__metatable', metatable is -2 and class is -3
        lua_pushvalue(L, -3); //copy 'class' that is in -3. now class is -1, '__metatable' is -2, metatable is -3 and class is -4
        /*
        -> QuickDebug::DumpLua(L);
        [Dump: -5] LUA_TNIL
        [Dump: -4] LUA_TTABLE THECLASS;
        [Dump: -3] LUA_TTABLE metatable_obj;
        [Dump: -2] LUA_TSTRING __metatable;
        [Dump: -1] LUA_TTABLE THECLASS;
        [Dump: 0] LUA_TNIL
        [Dump: 1] LUA_TTABLE THECLASS;
        [Dump: 2] LUA_TTABLE metatable_obj;
        [Dump: 3] LUA_TSTRING __metatable;
        [Dump: 4] LUA_TTABLE THECLASS;        
        */
        lua_settable(L, -3); //metatable.__metatable = class
        
        lua_pushstring(L, "__index"); //Now this is -1 '__index', metatable is -2 and class is -3
        lua_pushvalue(L, -3); //copy 'class' that is in -3. now class is -1, '__metatable' is -2, metatable is -3 and class is -4
        lua_settable(L, -3);  //metatable.__index = class
        lua_pop(L, 2);
        //Class created~
    };

    template<typename RetType,typename ... Types> static void RegisterClassLambdaMethod(lua_State *L,std::string name,std::string methodName,std::function<RetType(Types ... args)> func){
        lua_getglobal(L, name.c_str());
        LambdaClassRegister(L,methodName,-2,func);
        lua_pop(L, 1);
    }


    template<typename RetType,typename ... Types,typename ... Otps> static void RegisterClassLambdaMethod(lua_State *L,std::string name,std::string methodName,std::function<RetType(Types ... args)> func,Otps ...optArgs){
        lua_getglobal(L, name.c_str());
        LambdaClassRegister(L,methodName,-2,func,optArgs...);
        lua_pop(L, 1);
    }


    template<typename RetType,typename ClassObj,typename ... Types> static void RegisterClassMethod(lua_State *L,std::string name,std::string methodName,RetType (ClassObj::*func)(Types ... args)){
        lua_getglobal(L, name.c_str());
        if (!lua_istable(L, -1)) {
            return;
        }
        int top = lua_gettop (L);
        internal_register<RetType,ClassObj,Types...>::LambdaRegisterStackOpt(L,methodName,top,func);
    };


    template<typename RetType,typename ClassObj,typename ... Types,typename ... Otps> static void RegisterClassMethod(lua_State *L,std::string name,std::string methodName,RetType (ClassObj::*func)(Types ... args),Otps ...optArgs){
        lua_getglobal(L, name.c_str());
        int top = lua_gettop (L);
        internal_register<RetType,ClassObj,Types...>::LambdaRegisterStackOpt(L,methodName,top,func,optArgs...);
        lua_pop(L, 1);
    };



    static void MakeFieldToLuaHandler(lua_State *L,T1 *obj,int (*cind)(lua_State *L),int (*nind)(lua_State *L)){
        lua_newtable(L);
        lua_pushstring(L,"__self");
        T1 **usr = static_cast<T1**>(lua_newuserdata(L, sizeof(T1)));
        *usr = obj;
        lua_settable(L,-3);

        lua_newtable(L);
        int ps = lua_gettop (L);
        lua_pushcfunction(L, cind);
        lua_setfield(L, ps, "__index");
        lua_pushcfunction(L, nind);
        lua_setfield(L, ps, "__newindex");
        lua_setmetatable(L, -2);
        lua_settable(L,-3);
    }

    template<typename mType,typename T2> static int RegisterField(lua_State *L,std::string luaname,mType T2::*fieldaddr){
        std::map<std::string,mType T2::*> &fieldData = FieldToLuaHandler<T1,mType>::getAddr();
        fieldData[luaname] = (mType T2::*)fieldaddr;
        return 1;
    }


};

//obj specific

template<typename T1> struct MakeLuaObject{
    static int Make(lua_State *L, T1* obj, std::string name){
        if (obj == nullptr){
            lua_pushnil(L);
            return 1;
        }
        ClassRegister<T1>::RegisterObject(L, name, obj, true);
        return 1;
    };
};


template<> struct GenericLuaReturner<BleCharacteristicsHandler*>{
    static void Ret(BleCharacteristicsHandler* vr,lua_State *L,bool forceTable = false){
        MakeLuaObject<BleCharacteristicsHandler>::Make(L, vr, "BleCharacteristicsHandler");
    };
};



template<> struct GenericLuaGetter<BleCharacteristicsHandler*> {
    static inline BleCharacteristicsHandler* Call(bool &hasArgError, lua_State *L, int stackPos = -1, bool pop = true) {

        if (!lua_istable(L, stackPos)) {
            hasArgError = true;
            const char* function_name = lua_tostring(L, lua_upvalueindex(1));
            luaL_error(L, "Expected a table value on parameter %d of function %s", lua_gettop(L), function_name);
            return nullptr;
        }
        if (!lua_istable(L, stackPos)){
             hasArgError = true;
            const char* function_name = lua_tostring(L, lua_upvalueindex(1));
            luaL_error(L, "Expected a table value on parameter %d of function %s", lua_gettop(L), function_name);
            return nullptr;
        }

        lua_getfield(L, -2, "__self");
        BleCharacteristicsHandler** sp = (BleCharacteristicsHandler**)lua_touserdata(L,-1);
        if (!sp){
            luaL_error(L, "Expected a lua object");
            return nullptr;
        }
        lua_pop(L, 1);
        lua_getfield(L, -2, "type");
          
        std::string otherType = std::string(lua_tostring(L, -1));
        lua_pop(L, 1);

        if (otherType != "BleCharacteristicsHandler"){
            luaL_error(L, "Type mismatched, expecting 'BleCharacteristicsHandler' instead got %s", otherType.c_str());
            return nullptr;
        }

        if (*sp == nullptr){
            luaL_error(L, "Null userdata in to the object");
            return nullptr;
        }

        if (pop) {
            lua_pop(L, 1);
        }  
        return  (*sp);
    }
};


           
              
            

template<> struct GenericLuaReturner<Batata*>{
    static void Ret(Batata* vr,lua_State *L,bool forceTable = false){
        MakeLuaObject<Batata>::Make(L, vr, "Batata");
    };
};



template<> struct GenericLuaGetter<Batata*> {
    static inline Batata* Call(bool &hasArgError, lua_State *L, int stackPos = -1, bool pop = true) {

        if (!lua_istable(L, stackPos)) {
            hasArgError = true;
            const char* function_name = lua_tostring(L, lua_upvalueindex(1));
            luaL_error(L, "Expected a table value on parameter %d of function %s", lua_gettop(L), function_name);
            return nullptr;
        }
        if (!lua_istable(L, stackPos)){
             hasArgError = true;
            const char* function_name = lua_tostring(L, lua_upvalueindex(1));
            luaL_error(L, "Expected a table value on parameter %d of function %s", lua_gettop(L), function_name);
            return nullptr;
        }

        lua_getfield(L, -2, "__self");
        Batata** sp = (Batata**)lua_touserdata(L,-1);
        if (!sp){
            luaL_error(L, "Expected a lua object");
            return nullptr;
        }
        lua_pop(L, 1);
        lua_getfield(L, -2, "type");
          
        std::string otherType = std::string(lua_tostring(L, -1));
        lua_pop(L, 1);

        if (otherType != "Batata"){
            luaL_error(L, "Type mismatched, expecting 'Batata' instead got %s", otherType.c_str());
            return nullptr;
        }

        if (*sp == nullptr){
            luaL_error(L, "Null userdata in to the object");
            return nullptr;
        }

        if (pop) {
            lua_pop(L, 1);
        }  
        return  (*sp);
    }
};
