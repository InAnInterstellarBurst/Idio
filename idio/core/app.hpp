/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

namespace Idio
{
	struct ApplicationInfo 
	{
		std::string name;
		std::string prefPath;
		std::unique_ptr<Logger> gameLogger;
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
	void run(App& app, std::string name)
	{
		char* prefpath = SDL_GetPrefPath("idio", name.c_str());
		if(prefpath == nullptr) {
			std::cout << "[Init error]: " << SDL_GetError() << std::endl;
			return;
		}

		ApplicationInfo appInfo{
			.name = std::move(name),
			.prefPath = prefpath,
			.gameLogger = std::make_unique<Logger>(appInfo.name, appInfo)
		};

		SDL_free(prefpath);
		
		s_EngineLogger = std::make_unique<Logger>("Idio", appInfo);

		app.appInfo = &appInfo;
		app.init();
		bool open = true;
		while(open) {
			s_EngineLogger->critical("Hi {}", 3);
			s_EngineLogger->trace("Hi {}", 3);
			
			app.tick();

			poll_evts(app,
				[&](const QuitEvent& qe) -> bool { open = false; return true; },
				[](const WindowClosedEvent& wce) -> bool { return false; }
			);
		}

		app.deinit();
	}

}