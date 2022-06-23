/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include "core/app.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

void reopen_console()
{
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	HANDLE stdhdl = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD cflags = 0;
	GetConsoleMode(stdhdl, &cflags);
	SetConsoleMode(stdhdl, ENABLE_VIRTUAL_TERMINAL_PROCESSING | cflags);
}

int WINAPI WinMain(HINSTANCE p0, HINSTANCE p1, LPSTR p2, int p3)
{
#if ID_DEBUG
	reopen_console();
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif
	std::span<char*> args(__argv, __argc);
	Idio::main(args);
#if ID_DEBUG
	system("Pause");
#endif
	return EXIT_SUCCESS;
}