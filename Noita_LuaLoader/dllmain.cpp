#include "stdafx.h"

DWORD WINAPI lpLuaLoader(LPVOID lpParam)
{
	//

	return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		CreateThread(0, 0, lpLuaLoader, 0, 0, 0);
	}

    return TRUE;
}

