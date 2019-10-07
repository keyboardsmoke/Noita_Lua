#pragma once
#include "targetver.h"

// Trim down the WinAPI crap. We also don't want WinGDI.h
// to interfere with our WGL wrapper declarations.
#define NOGDI
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

#define FROM_IDA_ADDRESS(x) (((x) - 0x400000) + (uintptr_t)GetModuleHandle(nullptr))