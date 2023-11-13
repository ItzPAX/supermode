#pragma once
#define WIN32_LEAN_AND_MEAN

#pragma warning (disable : 4838)
#pragma warning (disable : 4309)
#pragma warning (disable : 4312)
#pragma warning (disable : 4244)
#pragma warning (disable : 4996)

#include <Windows.h>
#include <stdio.h>
#include <wingdi.h>
#include <dwmapi.h>
#include <TlHelp32.h>
#include <cstdint>
#include <thread>
#include <array>
#include <winternl.h>
#include <playsoundapi.h>
#include <codecvt>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <vector>
#include <sstream>
#include <unordered_map>

#define TRANSPARENT_COLOR RGB(105, 50, 45)

namespace globals
{
	bool init_overlay = false;

	HANDLE game;
	DWORD pid{};

	HWND hwQuickassist;

	int overlay_w, overlay_h;
	HANDLE overlay_handle, slowupdate_handle;
}

#include "render.h"
#include "menu.h"