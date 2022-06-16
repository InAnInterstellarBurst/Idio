/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include <variant>

namespace
{
	template<typename... Ts>
	struct overloaded : Ts...
	{
		using Ts::operator()...;
	};

	template<typename... Ts>
	overloaded(Ts...) -> overloaded<Ts...>;
}

namespace Idio
{
	struct QuitEvent {};
	struct WindowClosedEvent 
	{
		uint32_t id;
	};


	using Event = std::variant<QuitEvent, WindowClosedEvent>;
	
	template<typename... Handlers>
	auto evt_handler(const Event& e, Handlers&&... h)
	{
		return std::visit(overloaded{ std::forward<Handlers>(h)... }, e);
	}

	inline void post_quit_evt()
	{
		SDL_Event quit = { .type = SDL_QUIT };
		SDL_PushEvent(&quit);
	}

	template<class App, typename... Handlers>
	void poll_evts(App& a, Handlers&&... h)
	{
		SDL_Event sdlEvt;
		while(SDL_PollEvent(&sdlEvt)) {
			Event translatedEvent;
			switch(sdlEvt.type) {
			case SDL_QUIT:
				translatedEvent = QuitEvent{};
				break;
			default:
				translatedEvent = WindowClosedEvent{ 3 };
				break;
			}

			bool handled = evt_handler(translatedEvent, std::forward<Handlers>(h)...);
			if(!handled) {
				a.event_proc(translatedEvent);
			}
		}
	}
}