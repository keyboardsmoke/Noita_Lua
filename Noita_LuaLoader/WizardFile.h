#pragma once

namespace wizard
{
	extern __declspec(noinline) void GetFiles(std::vector<std::string>* iter, int depth, std::string str);
	extern __declspec(noinline) bool GetFileContents(const char* filename, std::string* data);
}