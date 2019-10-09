#include "stdafx.h"
#include "WizardFile.h"

namespace wizard
{
	struct WizardClass
	{
		virtual void unk0() = 0;			// 0000
		virtual void unk1() = 0;			// 0004
		virtual void unk2() = 0;			// 0008
		virtual void unk3() = 0;			// 000C
		virtual void unk4() = 0;			// 0010
		virtual void unk5() = 0;			// 0014
		virtual void unk6() = 0;			// 0018
		virtual void unk7() = 0;			// 001C
		virtual void unk8() = 0;			// 0020
		virtual void* GetFileSystem() = 0;	// 0024
	};

	typedef WizardClass* (*GetWizardClass_t)();
	typedef int(__thiscall* GetWizardFileContents_t)(void* p_this, void*, void** buffer);
	typedef void* (__thiscall* GetFiles_t)(void* p_this, std::vector<std::string>* iter, int depth, std::string* str);
	typedef int(__cdecl* GetIteratorValue_t)(void* value, void* iter);

	__declspec(noinline) void GetIteratorValue(void* p, void* iter)
	{
		static GetIteratorValue_t GetIteratorValueNative = (GetIteratorValue_t)FROM_IDA_ADDRESS(0x00469961);

		GetIteratorValue(p, iter);
	}

	__declspec(noinline) void GetFiles(std::vector<std::string>* iter, int depth, std::string str)
	{
		static GetWizardClass_t GetWizardClass = (GetWizardClass_t)FROM_IDA_ADDRESS(0x0042F702);
		static GetFiles_t GetFilesNative = (GetFiles_t)FROM_IDA_ADDRESS(0x00409DE5);

		WizardClass* wz = GetWizardClass();
		void* fs = wz->GetFileSystem();

		GetFilesNative(fs, iter, depth, &str);
	}

	__declspec(noinline) bool GetFileContents(const char* filename, std::string* data)
	{
		static GetWizardClass_t GetWizardClass = (GetWizardClass_t)FROM_IDA_ADDRESS(0x0042F702);
		static GetWizardFileContents_t GetWizardFileContents = (GetWizardFileContents_t)FROM_IDA_ADDRESS(0x00407522);

		std::string str = filename;
		WizardClass* cls = GetWizardClass();
		volatile void* fs = cls->GetFileSystem();

		if (GetWizardFileContents((void*)fs, &str, (void**)data) == 1)
		{
			return true;
		}

		return false;
	}
}