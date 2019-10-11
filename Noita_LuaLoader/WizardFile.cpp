#include "stdafx.h"
#include "WizardFile.h"

namespace wizard
{
	typedef IPlatform* (*GetPlatform_t)();
	typedef int(__thiscall* GetWizardFileContents_t)(void* p_this, std::string*, std::string* buffer);
	typedef void* (__thiscall* GetFiles_t)(FileSystem* p_this, std::vector<std::string>* iter, int depth, std::string* str);

	__declspec(noinline) FileSystem* GetFileSystem()
	{
		static GetPlatform_t GetPlatform = (GetPlatform_t)FROM_IDA_ADDRESS(0x0042F64E); // Oct 10

		return GetPlatform()->GetFileSystem();
	}

	__declspec(noinline) void GetFiles(std::vector<std::string>* iter, int depth, std::string str)
	{
		static GetFiles_t GetFilesNative = (GetFiles_t)FROM_IDA_ADDRESS(0x00409C37); // Oct 10

		GetFilesNative(GetFileSystem(), iter, depth, &str);
	}

	__declspec(noinline) bool GetFileContents(const char* filename, std::string* data)
	{
		static GetWizardFileContents_t GetWizardFileContents = (GetWizardFileContents_t)FROM_IDA_ADDRESS(0x0040754F); // Oct 10

		std::string str = filename;

		DWORD len = strlen(filename);
		_asm mov esi, len;

		if (GetWizardFileContents(GetFileSystem(), &str, data) == 1)
		{
			return true;
		}

		return false;
	}
}