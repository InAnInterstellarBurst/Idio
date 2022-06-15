/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "app.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void reopen_console()
{
	AllocConsole();
	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	DWORD cflags = 0;
	GetConsoleDisplayMode(&cflags);
	SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),
		ENABLE_VIRTUAL_TERMINAL_PROCESSING | cflags);
}

int WINAPI WinMain(HINSTANCE p0, HINSTANCE p1, LPSTR p2, int p3)
{
#if ID_DEBUG
	reopen_console();
#endif
	std::span<char*> args(__argv, __argc);
	Idio::main(args);
#if ID_DEBUG
	system("Pause");
#endif
	return EXIT_SUCCESS;
}