/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include "event.hpp"

#include <SDL.h>

namespace Idio
{
	Event poll_evts()
	{
		SDL_Event evt;
		while(SDL_PollEvent(&evt)) {
			switch(evt.type) {
			case SDL_QUIT:
				return QuitEvent{};
			default:
				return WindowClosedEvent{3};
			}
		}

		return WindowClosedEvent{3};
	}

	void post_quit_evt()
	{
		SDL_Event quit = { .type = SDL_QUIT };
		SDL_PushEvent(&quit);
	}
}