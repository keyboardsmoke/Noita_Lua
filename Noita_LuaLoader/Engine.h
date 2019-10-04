#pragma once

typedef uintptr_t lua_State;

class LuaStateMgr
{
public:
	// vftable
	virtual ~LuaStateMgr() = 0;
	virtual void unk() = 0;
	virtual void DoFile() = 0;

	// data
	char*			name;			// 0004
	unsigned char	_unk000[0x14];	// 0008
	lua_State*		state;			// 001C
};

// Game APIs
typedef LuaStateMgr* (*GetGlobalLuaManager_t)();

// Lua APIs
typedef int (*lua_pcall_t)(lua_State *L, int nargs, int nresults, int errfunc);

typedef int (*luaL_loadbuffer_t)(lua_State *L,
	const char *buff,
	size_t sz,
	const char *name);

typedef int (*luaL_loadbufferx_t)(lua_State *L,
	const char *buff,
	size_t sz,
	const char *name,
	const char *mode);