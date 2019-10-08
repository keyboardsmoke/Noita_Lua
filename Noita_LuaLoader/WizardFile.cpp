#include "stdafx.h"
#include "WizardFile.h"

/*
struct wizard_std_string
{
	wizard_std_string()
	{
		memset(buffer, 0, sizeof(buffer));
		size = 0;
		capacity = 15;
	}

	unsigned char buffer[0x10];
	int size;
	unsigned int capacity;
};*/

/* void *file_buffer; // [esp+30h] [ebp-28h]
  int sz; // [esp+40h] [ebp-18h]
  unsigned int v18; // [esp+44h] [ebp-14h]*/

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

typedef void (__thiscall *make_std_string_t)(void*, const char* c_str, size_t len);
typedef WizardClass* (*GetWizardClass_t)();
typedef int (__thiscall *GetWizardFileContents_t)(void* p_this, void*, void** buffer);
typedef std::string* (__cdecl* GetWizardFileContents_Simple_t)(const char* filename, uint32_t* buffer);

__declspec(noinline) bool GetWizardFileContents(const char* filename, std::string* data)
{
	volatile GetWizardClass_t GetWizardClass = (GetWizardClass_t)FROM_IDA_ADDRESS(0x0042F702);
	volatile GetWizardFileContents_t GetWizardFileContents = (GetWizardFileContents_t)FROM_IDA_ADDRESS(0x00407522);
	
	std::string str = filename;
	WizardClass* cls = GetWizardClass();
	volatile void* fs = cls->GetFileSystem();

	if (GetWizardFileContents((void*)fs, &str, (void**)data) == 1)
	{
		return true;
	}

	return false;
}