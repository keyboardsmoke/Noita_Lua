#pragma once

namespace wizard
{
	struct IFileDevice
	{
		virtual void unk00() = 0;
		virtual void unk01() = 0;
		virtual void unk02() = 0;
		virtual void unk03() = 0;
		virtual void GetFullPath() = 0;
		virtual void unk05() = 0;
		virtual void unk06() = 0;
		virtual void unk07() = 0;
		virtual void unk08() = 0;
		virtual void unk09() = 0;
		virtual void unk10() = 0;
		
		//
	};

	struct DiskFileDevice : public IFileDevice
	{
		std::string mReadRootPath;		// 0004

		// There's other stuff but who cares?
	};

	struct WizardPakEntry
	{
		uint32_t hashThing;
		uint32_t unk;
		uint32_t stringLength;
		char filename[];
	};

	struct WizardPakFileList
	{
		uint32_t null_0;				// 0000 (0)
		uint32_t num_entries;			// 0004 (9120)
		uint32_t unk0;					// 0008 (0x74a44)
		uint32_t unk1;					// 000C (0)
		WizardPakEntry first;			// 0010
	};

	struct WizardPakFileDevice : public IFileDevice
	{
		int32_t _unk;					// 0004
		WizardPakFileList* list;		// 0008
	};

	struct FSContainer
	{
		WizardPakFileDevice* pak;
	};

	struct FileSystem
	{
		FSContainer* container;
	};

	struct IPlatform
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
		virtual FileSystem* GetFileSystem() = 0;	// 0024
	};

	extern __declspec(noinline) FileSystem* GetFileSystem();
	extern __declspec(noinline) void GetFiles(std::vector<std::string>* iter, int depth, std::string str);
	extern __declspec(noinline) bool GetFileContents(const char* filename, std::string* data);
}