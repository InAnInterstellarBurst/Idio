/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "event.hpp"
#include "logger.hpp"

namespace Idio
{
	extern void main(const std::span<char*>& args);

	struct ApplicationInfo 
	{
		std::string name;
		std::string prefPath;
	};

	template<class T>
	concept Application = requires(T t, const Event& e)
	{
		t.init();
		t.tick();
		t.deinit();
		t.event_proc(e);
		requires std::same_as<decltype(t.appInfo), const ApplicationInfo*>;
	};

	
	template<Application App>
	bool run(App& app, std::string name)
	{
		char* prefpath = SDL_GetPrefPath("Idio", name.c_str());
		if(prefpath == nullptr) {
			std::cout << "[Init error]: " << SDL_GetError() << std::endl;
			return false;
		}

		const ApplicationInfo appInfo{
			.name = std::move(name),
			.prefPath = std::string(prefpath)
		};

		SDL_free(prefpath);
		bool open = true;

		app.appInfo = &appInfo;
		app.init();
		while(open) {
			app.tick();

			poll_evts(app,
				[&](const QuitEvent& qe) -> bool { open = false; return true; },
				[](const WindowClosedEvent& wce) -> bool { return false; }
			);
		}

		app.deinit();
		return true;
	}
}