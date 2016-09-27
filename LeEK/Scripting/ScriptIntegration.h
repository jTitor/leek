#pragma once
extern "C" {
	//directly including Lua in project; does this affect builds on other machines?
	//doesn't need to build if libraries are installed, but will these lines still find the headers in the project?
#include "Libraries/Lua/lua.h"
#include "Libraries/Lua/lualib.h"
#include "Libraries/Lua/lauxlib.h"
}

#include "Libraries/LuaBridge/LuaBridge.h"

#include "DebugUtils/Assertions.h"

namespace LeEK
{
	class IScriptInterface
	{
	public:
		virtual ~IScriptInterface() {};
	};


	class LuaInterface : IScriptInterface
	{
	private:
		lua_State *lua_state;

		void luaGetError()
		{
			const std::string message = lua_tostring(lua_state, -1);
			lua_pop(lua_state, 1);
			throw std::runtime_error(message);
		}

		LuaInterface(const LuaInterface &)
		{
			L_ASSERT("Illegal copying of Lua interface!");
		}
		LuaInterface &operator=(const LuaInterface &)
		{
			L_ASSERT("Illegal assignment of Lua interface!");
		}
	public:
		LuaInterface()
		{
			lua_state = NULL;
			if (!(lua_state = luaL_newstate()))
				throw std::runtime_error("error: luaL_newstate() failed");

			luaL_openlibs(lua_state);
		}

		lua_State* LuaState()
		{
			return lua_state;
		}
		/*
		void RegisterFunction(const std::string &name, int function(lua_State *))
		{
			lua_register(lua_state, name.c_str(), function);
		}
		*/

		void ExecuteFile(const std::string &path)
		{
			if (luaL_dofile(lua_state, path.c_str()))
			{
				luaGetError();
			}
		}

		//Looks a bit strange, but this makes it pretty easy to register a function.
		#define LuaRegisterFunction(l, name, f) (l.RegisterFunction(name, LeEK::Details::lua::LuaFunctionWrapper<decltype(f), f>))

		~LuaInterface()
		{
			lua_close(lua_state);
		}
	};
}