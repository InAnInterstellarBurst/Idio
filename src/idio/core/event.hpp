/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IDIO_CORE_EVENT_H
#define IDIO_CORE_EVENT_H

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

namespace idio
{
	class Application;

	struct NoEvent
	{
	};
	struct QuitEvent
	{
	};
	struct WindowClosedEvent
	{
		uint32_t id;
	};

	struct WindowResizeEvent
	{
		uint32_t id;
		int width, height;
	};

	struct WindowMinimiseEvent
	{
		uint32_t id;
		bool minimised;
	};

	using Event = std::variant<NoEvent, QuitEvent,
		WindowClosedEvent, WindowMinimiseEvent, WindowResizeEvent>;

	template<typename... Handlers>
	auto evt_handler(const Event &e, Handlers &&...h)
	{
		return std::visit(overloaded { [](const NoEvent &no) -> bool { return true; }, std::forward<Handlers>(h)... }, e);
	}

	inline Event translate_evt(SDL_Event sdlEvt)
	{
		Event translatedEvent = NoEvent {};
		switch(sdlEvt.type) {
		case SDL_QUIT:
			translatedEvent = QuitEvent {};
			break;
		case SDL_WINDOWEVENT:
			switch(sdlEvt.window.event) {
			case SDL_WINDOWEVENT_CLOSE:
				translatedEvent = WindowClosedEvent { .id = sdlEvt.window.windowID };
				break;
			case SDL_WINDOWEVENT_RESTORED:
				translatedEvent = WindowMinimiseEvent {
					.id = sdlEvt.window.windowID,
					.minimised = false
				};
				break;
			case SDL_WINDOWEVENT_MINIMIZED:
				translatedEvent = WindowMinimiseEvent {
					.id = sdlEvt.window.windowID,
					.minimised = true
				};
				break;
			case SDL_WINDOWEVENT_RESIZED:
				translatedEvent = WindowResizeEvent {
					.id = sdlEvt.window.windowID,
					.width = sdlEvt.window.data1,
					.height = sdlEvt.window.data2
				};
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}

		return translatedEvent;
	}
}

#endif
