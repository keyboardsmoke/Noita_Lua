#include "stdafx.h"
#include "WizardFile.h"

namespace wizard
{
	typedef IPlatform* (*GetPlatform_t)();
	typedef int(__thiscall* GetWizardFileContents_t)(void* p_this, void*, void** buffer);
	typedef void* (__thiscall* GetFiles_t)(FileSystem* p_this, std::vector<std::string>* iter, int depth, std::string* str);

	__declspec(noinline) FileSystem* GetFileSystem()
	{
		static GetPlatform_t GetPlatform = (GetPlatform_t)FROM_IDA_ADDRESS(0x0042F702);

		return GetPlatform()->GetFileSystem();
	}

	__declspec(noinline) void GetFiles(std::vector<std::string>* iter, int depth, std::string str)
	{
		static GetFiles_t GetFilesNative = (GetFiles_t)FROM_IDA_ADDRESS(0x00409DE5);

		GetFilesNative(GetFileSystem(), iter, depth, &str);
	}

	__declspec(noinline) bool GetFileContents(const char* filename, std::string* data)
	{
		static GetWizardFileContents_t GetWizardFileContents = (GetWizardFileContents_t)FROM_IDA_ADDRESS(0x00407522);

		std::string str = filename;

		if (GetWizardFileContents(GetFileSystem(), &str, (void**)data) == 1)
		{
			return true;
		}

		return false;
	}
}