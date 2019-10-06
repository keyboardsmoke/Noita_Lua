#include "stdafx.h"
#include "MinHook.h"
#include "Engine.h"

lua_pcall_t plua_pcall = nullptr;
luaL_loadbufferx_t pluaL_loadbufferx = nullptr;
luaL_loadbuffer_t pluaL_loadbuffer = nullptr;

int new_luaL_loadbufferx(lua_State *L, const char *buff, size_t sz, const char *name, const char *mode)
{
	// log
	std::stringstream filename;
	filename << "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Noita\\mod\\dump\\" << name << ".lua";

	std::ofstream file(filename.str(), std::ifstream::binary);
	
	if (file.is_open())
	{
		file.write(buff, sz);
		file.close();
	}

	return pluaL_loadbufferx(L, buff, sz, name, mode);
}

int new_luaL_loadbuffer(lua_State *L, const char *buff, size_t sz, const char *name)
{
	// log
	std::stringstream filename;
	filename << "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Noita\\mod\\dump\\" << name << ".lua";

	std::ofstream file(filename.str(), std::ifstream::binary);

	if (file.is_open())
	{
		file.write(buff, sz);
		file.close();
	}

	return pluaL_loadbuffer(L, buff, sz, name);
}

bool SetupDumpHook()
{
	HMODULE hLua = LoadLibraryA("lua51.dll");
	if (hLua == nullptr)
		return false;

	plua_pcall = (lua_pcall_t)GetProcAddress(hLua, "lua_pcall");
	FARPROC fpluaL_loadbufferx = GetProcAddress(hLua, "luaL_loadbufferx");
	FARPROC fpluaL_loadbuffer = GetProcAddress(hLua, "luaL_loadbuffer");

	if (!plua_pcall || 
		!fpluaL_loadbufferx ||
		!fpluaL_loadbuffer)
	{
		return false;
	}

	auto status = MH_Initialize();
	if (status != MH_OK)
		return false;

	status = MH_CreateHook((LPVOID)fpluaL_loadbufferx, (LPVOID)new_luaL_loadbufferx, (LPVOID*)&pluaL_loadbufferx);
	if (status != MH_OK)
		return false;

	status = MH_CreateHook((LPVOID)fpluaL_loadbuffer, (LPVOID)new_luaL_loadbuffer, (LPVOID*)&pluaL_loadbuffer);
	if (status != MH_OK)
		return false;

	status = MH_EnableHook((LPVOID)fpluaL_loadbufferx);
	if (status != MH_OK)
		return false;

	status = MH_EnableHook((LPVOID)fpluaL_loadbuffer);
	if (status != MH_OK)
		return false;

	return true;
}

bool LoadLuaScript(LuaStateMgr* mgr)
{
	std::stringstream filename;
	filename << "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Noita\\mod\\main.lua";

	std::ifstream file(filename.str(), std::ifstream::binary);
	
	// 

	if (file.is_open())
	{
		std::vector<char> buffer(std::istreambuf_iterator<char>(file), {});
		file.close();

		if (pluaL_loadbuffer(mgr->state, buffer.data(), buffer.size(), "") != 0)
			return false;

		if (plua_pcall(mgr->state, 0, -1, 0) != 0)
			return false;

		return true;
	}

	return false;
}

DWORD WINAPI lpLuaLoader(LPVOID lpParam)
{
	GetGlobalLuaManager_t pGetGlobalLuaManager = (GetGlobalLuaManager_t)((uintptr_t)GetModuleHandle(nullptr) + 0xA2B350);

	LuaStateMgr* mgr = nullptr;
	while (mgr == nullptr)
	{
		mgr = pGetGlobalLuaManager();
		Sleep(10);
	}

	while (true)
	{
		if (GetAsyncKeyState(VK_INSERT) & 1)
		{
			if (!LoadLuaScript(mgr))
			{
				MessageBoxA(0, "Failed to load lua script.", "ERROR", MB_OK);
			}
		}

		Sleep(100);
	}

	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		// Setup dumper hook
		if (!SetupDumpHook())
		{
			MessageBoxA(0, "Error: Unable to initialize lua dump hook...", "ERROR", MB_OK);
			ExitProcess(0);
		}

		CreateThread(0, 0, lpLuaLoader, 0, 0, 0);
	}

    return TRUE;
}

