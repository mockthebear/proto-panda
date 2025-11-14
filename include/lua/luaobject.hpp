#pragma once 

#include "lua/LuaWrapper.h"
#include <Arduino.h>

class Batata{
    public:
    Batata(){count = 0;};
    ~Batata(){
        Serial.printf("THE BATATA IS DEAD MAN\n");
    };
    uint32_t Get(){return count;};
    void Set(uint32_t c){ count = c;};
    uint32_t Sum(uint32_t a, uint32_t b){ count = a+b; return count;};
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

        template <typename T> static int Destroy(lua_State *L){
            T* part = LuaCaller::GetSelf<T>(L);
            if (!part){
                return 0;
            }
            delete part;

            lua_pushnil(L);

            while (lua_next(L, 1) != 0) {
                // Key is at -2, value at -1
                lua_pop(L, 1);  // Remove value, keep key for next iteration
                lua_pushvalue(L, -1);  // Copy key to top
                lua_pushnil(L);  // Push nil value
                lua_settable(L, 1);  // table[key] = nil
            }
            
            // Remove metatable
            lua_pushnil(L);
            lua_setmetatable(L, 1);
            
            return 0;
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
        std::string field = lua_tostring(L,-2);
        bool hasErr = false;
        T1 *self = LuaCaller::GetSelf<T1>(L, -2);
        if (!self){
            return 0;
        }
        std::map<std::string,ObjT T1::*> &fieldData = FieldToLuaHandler<T1,ObjT>::getAddr();
        if (fieldData[field]){
            ObjT         data = GenericLuaGetter<ObjT>::Call(hasErr, L,-1);
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



    static void RegisterObject(lua_State *L, std::string name, T1 *obj, bool Force=true){
        if (!obj){
            lua_pushstring(L, "[LUA] on RegisterObject, null reference.");
            lua_error(L);
            return;
        }


        if (Force){
            lua_getglobal(L, name.c_str());
            luaL_checktype(L, 1, LUA_TTABLE);
        }
        
        // Create the object table
        lua_newtable(L);
        
        // Set basic properties
        lua_pushstring(L, "id");
        lua_pushnumber(L, (uint64_t)obj);
        lua_settable(L, -3);

        lua_pushstring(L, "type");
        lua_pushstring(L, name.c_str());
        lua_settable(L, -3);
        
        lua_pushstring(L, "data");
        
        // Set up metatable and indexing
        ClassRegister<T1>::MakeFieldToLuaHandler(L, obj, IndexerHelper<T1>::Index, IndexerHelper<T1>::Newindex);

        lua_pushvalue(L, 1);
        lua_setmetatable(L, -2);

        lua_pushvalue(L, 1);
        lua_setfield(L, 1, "__index");
        
        // Create userdata for the object
        T1 **usr = static_cast<T1**>(lua_newuserdata(L, sizeof(T1*)));
        *usr = obj;

        lua_getglobal(L, name.c_str());
        lua_setmetatable(L, -2);
        lua_setfield(L, -2, "__self");

        // The created object table is now on top of the stack
        // No insertion into __REFS anymore
        
        // If we need to return the object, it's already on top of the stack
    };

     static void RegisterClassType(lua_State *L,std::string name,
		 std::function<T1*(lua_State*)> makerF = std::function<T1*(lua_State*)>(),
                                      LuaCFunctionLambda *gc_func = nullptr){

        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, name.c_str());
        int methods = lua_gettop(L);
        lua_newtable(L);
        int methodsTable = lua_gettop(L);

        static LuaCFunctionLambda Flambb = [name,makerF](lua_State* Ls) -> int{
			T1 *obj;
			if (makerF) {
				obj = makerF(Ls);
			}else{
				obj = new T1();
			}
			if (!obj){
                return 0;
			}
			RegisterObject(Ls,name,obj,false);
            return 1;
        };

;
        LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
        (*baseF) = &Flambb;
        lua_pushcclosure(L, LuaCaller::BaseEmpty<1>,1);
        lua_setfield(L, methodsTable, "__call");
        lua_setmetatable(L, methods);
        luaL_newmetatable(L, name.c_str());
        int metatable = lua_gettop(L);
        lua_pushvalue(L, methods);
        lua_setfield(L, metatable, "__metatable");
        lua_pushvalue(L, methods);
        lua_setfield(L, metatable, "__index");
        lua_pop(L, 2);
        lua_getglobal(L, name.c_str());
        if (gc_func && *gc_func){
            baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
            (*baseF) = &(*gc_func);
            lua_pushcclosure(L, LuaCaller::Base<1>,1);
        }else{
            lua_pushcfunction(L, LuaCaller::GC<T1>);
        }
        lua_setfield(L, -2, "__gc");
        lua_pop(L, 1);

        lua_getglobal(L, name.c_str());
        lua_pushcfunction(L, LuaCaller::Destroy<T1>);
        lua_setfield(L, -2,  "destroy");
        lua_pop(L, 1);
    };

static void RegisterClassVirtual(lua_State *L,std::string name,
		 std::function<T1*(lua_State*)> makerF, LuaCFunctionLambda *gc_func){

        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setglobal(L, name.c_str());
        int methods = lua_gettop(L);
        lua_newtable(L);
        int methodsTable = lua_gettop(L);

        static LuaCFunctionLambda Flambb = [name,makerF](lua_State* Ls) -> int{
			T1 *obj = nullptr;
			if (makerF) {
				obj = makerF(Ls);
			}
			RegisterObject(Ls,name,obj,false);
            return 1;
        };

        LuaCFunctionLambda** baseF = static_cast<LuaCFunctionLambda**>(lua_newuserdata(L, sizeof(LuaCFunctionLambda) ));
        (*baseF) = &Flambb;
        lua_pushcclosure(L, LuaCaller::BaseEmpty<1>,1);
        lua_setfield(L, methodsTable, "__call");
        lua_setmetatable(L, methods);
        luaL_newmetatable(L, name.c_str());
        int metatable = lua_gettop(L);
        lua_pushvalue(L, methods);
        lua_setfield(L, metatable, "__metatable");
        lua_pushvalue(L, methods);
        lua_setfield(L, metatable, "__index");
        lua_pop(L, 2);
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
