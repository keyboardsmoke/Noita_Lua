#include "stdafx.h"
#include "MinHook.h"
#include "Engine.h"
#include "WizardFile.h"

typedef bool(__thiscall* RunScriptUpdate_t)(void* p_this);
typedef int(__cdecl* GuiStartFrame_t)(lua_State* L);

lua_pcall_t plua_pcall = nullptr;
luaL_loadbufferx_t pluaL_loadbufferx = nullptr;
luaL_loadbuffer_t pluaL_loadbuffer = nullptr;
RunScriptUpdate_t pRunScriptUpdate = nullptr;
GuiStartFrame_t pGuiStartFrame = nullptr;

//char __thiscall sub_F54420(float *this)

int new_luaL_loadbufferx(lua_State *L, const char *buff, size_t sz, const char *name, const char *mode)
{
	// log
	std::stringstream filename;
	filename << "C:\\Noita\\mod\\dump\\" << name << ".lua";

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
	filename << "C:\\Noita\\mod\\dump\\" << name << ".lua";

	std::ofstream file(filename.str(), std::ifstream::binary);

	if (file.is_open())
	{
		file.write(buff, sz);
		file.close();
	}

	return pluaL_loadbuffer(L, buff, sz, name);
}

// data/scripts/items/drop_money16x.lua

void DumpScript(const char* name)
{
	std::string localFileName = "C:\\Noita\\mod\\dump\\" + std::string(name);

	std::filesystem::path localPath(localFileName);
	std::filesystem::path parentPath = localPath.parent_path();

	if (!std::filesystem::exists(parentPath))
	{
		std::filesystem::create_directories(parentPath);
	}

	std::string data;
	if (!GetWizardFileContents(name, &data))
	{
		MessageBoxA(0, "Failed", "ERROR", MB_OK);
		return;
	}
	else
	{
		//MessageBoxA(0, data.c_str(), "CONTENT", MB_OK);
	}

	std::ofstream file(localFileName, std::ifstream::binary);

	if (file.is_open())
	{
		file.write(data.data(), data.length());
		file.close();
	}
}

void RunDump()
{
	DumpScript("data/credits.txt");
	DumpScript("data/scripts/items/drop_money16x.lua");
	DumpScript("data/global/weather_config.xml");
	DumpScript("data/scripts/magic/symbol_world_spells.lua");
	DumpScript("data/scripts/magic/symbol_oneoff_spells.lua");
	DumpScript("data/global/worldstate_config.xml");

	//DumpScript("temptemp/data_wak_files.txt");

	ExitProcess(0);
}

bool __fastcall new_RunScriptUpdate(void* p_this)
{
	bool ret = pRunScriptUpdate(p_this);

	char buffer[2048] = { 0 };

	// DWORD TlsIndex = *(DWORD*)FROM_IDA_ADDRESS(0x014C79E0);
	// sprintf_s(buffer, "[%d]", TlsIndex);
	// MessageBoxA(0, buffer, "TLS INDEX", MB_OK);
	// 
	// DWORD dwTlsValue = *(DWORD*)(__readfsdword(0x2Cu) + 4 * TlsIndex);
	// sprintf_s(buffer, "[%d]", dwTlsValue);
	// MessageBoxA(0, buffer, "TLS VALUE", MB_OK);
	// 
	// DWORD dwTlsPtr = *(DWORD*)(dwTlsValue + 0x11C);
	// sprintf_s(buffer, "[%d]", dwTlsPtr);
	// MessageBoxA(0, buffer, "TLS PTR", MB_OK);

	RunDump();

	return ret;
}

int __cdecl new_GuiStartFrame(lua_State* L)
{
	// int __cdecl sub_E9A2D0(int a1)

	int r = pGuiStartFrame(L);

	

	return r;
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

	MH_STATUS status = MH_Initialize();
	if (status != MH_OK)
		return false;

	status = MH_CreateHook((LPVOID)fpluaL_loadbufferx, (LPVOID)new_luaL_loadbufferx, (LPVOID*)&pluaL_loadbufferx);
	if (status != MH_OK)
		return false;

	status = MH_CreateHook((LPVOID)fpluaL_loadbuffer, (LPVOID)new_luaL_loadbuffer, (LPVOID*)&pluaL_loadbuffer);
	if (status != MH_OK)
		return false;

	LPVOID lpRunScriptUpdate = (LPVOID) FROM_IDA_ADDRESS(0xF54420);
	LPVOID lpGuiStartFrame = (LPVOID)FROM_IDA_ADDRESS(0x43D2B2);// 0x428380);

	status = MH_CreateHook(lpRunScriptUpdate, (LPVOID)new_RunScriptUpdate, (LPVOID *)&pRunScriptUpdate);
	if (status != MH_OK)
		return false;

	status = MH_CreateHook(lpGuiStartFrame, (LPVOID)new_GuiStartFrame, (LPVOID*)&pGuiStartFrame);
	if (status != MH_OK)
		return false;

	status = MH_EnableHook((LPVOID)fpluaL_loadbufferx);
	if (status != MH_OK)
		return false;

	status = MH_EnableHook((LPVOID)fpluaL_loadbuffer);
	if (status != MH_OK)
		return false;
	
	status = MH_EnableHook(lpRunScriptUpdate);
	if (status != MH_OK)
		return false;

	status = MH_EnableHook(lpGuiStartFrame);
	if (status != MH_OK)
		return false;

	return true;
}

bool LoadLuaScript(LuaStateMgr* mgr)
{
	std::stringstream filename;
	filename << "C:\\Noita\\mod\\main.lua";

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
	GetGlobalLuaManager_t pGetGlobalLuaManager = (GetGlobalLuaManager_t)FROM_IDA_ADDRESS(0xE371B0);

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

