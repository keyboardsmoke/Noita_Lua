#include "pch.h"
#include <iostream>
#include <TlHelp32.h>

const char InjectedModule[] = "opengl32.dll";


bool EnableDebugPrivilege(bool Enable)
{
	bool Success = false;

	HANDLE hToken = NULL;

	DWORD ec = 0;

	do
	{
		// Open the process' token

		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
		{
			ec = GetLastError();
			_ASSERTE(!_T("OpenProcessToken() failed."));
			break;
		}


		// Lookup the privilege value 

		TOKEN_PRIVILEGES tp;

		tp.PrivilegeCount = 1;

		if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid))
		{
			ec = GetLastError();
			_ASSERTE(!_T("LookupPrivilegeValue() failed."));
			break;
		}


		// Enable/disable the privilege

		tp.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

		if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL))
		{
			ec = GetLastError();
			_ASSERTE(!_T("AdjustPrivilegeValue() failed."));
			break;
		}


		// Success 

		Success = true;

	} while (0);


	// Cleanup

	if (hToken != NULL)
	{
		if (!CloseHandle(hToken))
		{
			ec = GetLastError();
			_ASSERTE(!_T("CloseHandle() failed."));
		}
	}


	// Complete 

	return Success;

}

HMODULE GetRemoteModuleHandle(DWORD dwProcessId, const char* szModule)
{
	HANDLE tlh = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);

	MODULEENTRY32 modEntry;
	ZeroMemory(&modEntry, sizeof(MODULEENTRY32));

	modEntry.dwSize = sizeof(MODULEENTRY32);

	if (!Module32First(tlh, &modEntry))
	{
		return nullptr;
	}

	do
	{
		std::cout << "Module name [" << modEntry.szModule << "]" << std::endl;

		if (_stricmp(szModule, modEntry.szModule) == 0)
		{
			CloseHandle(tlh);

			return modEntry.hModule;
		}
	} 
	while (Module32Next(tlh, &modEntry));

	CloseHandle(tlh);

	return nullptr;
}

bool LoadModuleRemote(const PROCESS_INFORMATION& pi, const char* moduleName)
{
	HMODULE hLocalKernel32 = LoadLibrary("kernel32.dll");
	if (!hLocalKernel32)
	{
		std::cerr << "Unable to get the local address of kernel32.dll." << std::endl;
		return false;
	}

	FARPROC fpLoadLibraryA = GetProcAddress(hLocalKernel32, "LoadLibraryA");
	if (!fpLoadLibraryA)
	{
		std::cerr << "Unable to get the local address of LoadLibraryA." << std::endl;
		return false;
	}

	ULONG_PTR dwLoadLibraryAOffset = (ULONG_PTR)((ULONG_PTR)fpLoadLibraryA - (ULONG_PTR)hLocalKernel32);

	HMODULE hRemoteKernel32 = GetRemoteModuleHandle(pi.dwProcessId, "kernel32.dll");
	if (!hRemoteKernel32)
	{
		std::cerr << "Unable to get the remote address of kernel32.dll." << std::endl;
		return false;
	}

	LPVOID lpModuleString = VirtualAllocEx(pi.hProcess, nullptr, strlen(moduleName) + 1, MEM_COMMIT, PAGE_READWRITE);
	if (!lpModuleString)
	{
		std::cerr << "Unable to allocate module name for injection." << std::endl;
		return false;
	}

	DWORD dwNumberOfBytesWritten = 0;
	if (!WriteProcessMemory(pi.hProcess, lpModuleString, moduleName, strlen(moduleName), &dwNumberOfBytesWritten))
	{
		std::cerr << "Unable to write module name for injection." << std::endl;
		return false;
	}

	// pi.hProcess
	HANDLE hThread = CreateRemoteThread(pi.hProcess, nullptr, 0,
		(LPTHREAD_START_ROUTINE)((ULONG_PTR)hRemoteKernel32 + dwLoadLibraryAOffset), (LPVOID)lpModuleString, 0, nullptr);

	if (hThread == nullptr)
	{
		std::cerr << "Unable to create remote thread." << std::endl;
		return false;
	}

	return true;
}

int main()
{
	if (!EnableDebugPrivilege(true))
	{
		std::cerr << "Unable to aquire debug privilege." << std::endl;
		return -1;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess("noita.exe", nullptr, nullptr, nullptr, FALSE, DEBUG_ONLY_THIS_PROCESS, nullptr, nullptr, &si, &pi))
	{
		std::cerr << "Failed to open noita.exe..." << std::endl;
		return -1;
	}

	BOOL bTargetProcessArch = FALSE;
	BOOL bCallerProcessArch = FALSE;
	IsWow64Process(pi.hProcess, &bTargetProcessArch);
	IsWow64Process(GetCurrentProcess(), &bCallerProcessArch);
	if (bTargetProcessArch != bCallerProcessArch)
	{
		TerminateProcess(pi.hProcess, 0);

		std::cerr << "Incompatible target." << std::endl;
		return -1;
	}

	if (!DebugSetProcessKillOnExit(FALSE))
	{
		TerminateProcess(pi.hProcess, 0);

		std::cerr << "Unable to set process kill on exit to false." << std::endl;
		return -1;
	}

	/*
	if (!DebugActiveProcess(pi.dwProcessId))
	{
		TerminateProcess(pi.hProcess, 0);

		std::cerr << "Unable to debug active process. (" << GetLastError() << ")" << std::endl;
		return -1;
	}*/

	DEBUG_EVENT debugEvent = { 0 };

	do
	{
		if (!WaitForDebugEvent(&debugEvent, INFINITE))
		{
			DebugActiveProcessStop(pi.dwProcessId);
			TerminateProcess(pi.hProcess, 0);

			std::cerr << "Unable to wait for debug event." << std::endl;
			return -1;
		}

		bool shouldBail = false;

		switch (debugEvent.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:
		{
			if (!LoadModuleRemote(pi, InjectedModule))
			{
				TerminateProcess(pi.hProcess, 0);

				std::cerr << "Unable to load library into process." << std::endl;
				return -1;
			}
			else
			{
				std::cout << "Injected module successfully." << std::endl;
			}

			shouldBail = true;

			break;
		}
		case UNLOAD_DLL_DEBUG_EVENT:
		{
			if (!bTargetProcessArch)
			{
				DebugActiveProcessStop(pi.dwProcessId);
				TerminateProcess(pi.hProcess, 0);

				std::cerr << "Unload event captured before injecting module." << std::endl;
				return -1;
			}
		}
		case LOAD_DLL_DEBUG_EVENT:
		case CREATE_THREAD_DEBUG_EVENT:
		case CREATE_PROCESS_DEBUG_EVENT:
		{
			if (!ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE))
			{
				DebugActiveProcessStop(pi.dwProcessId);
				TerminateProcess(pi.hProcess, 0);

				std::cerr << "Unable to continue debug event." << std::endl;
				return -1;
			}

			break;
		}
		}

		if (shouldBail)
			break;
	}
	while (true);

	DebugActiveProcessStop(pi.dwProcessId);

	return 0;
}