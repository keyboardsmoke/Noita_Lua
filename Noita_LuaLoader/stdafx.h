#pragma once
#include "targetver.h"

// Trim down the WinAPI crap. We also don't want WinGDI.h
// to interfere with our WGL wrapper declarations.
#define NOGDI
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <windows.h>