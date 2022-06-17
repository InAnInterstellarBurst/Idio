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
		std::unique_ptr<Window> mainWindow;
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
	void run(App& app, const WindowCreateInfo& wci, std::string name)
	{
		char* prefpath = SDL_GetPrefPath("idio", name.c_str());
		if(prefpath == nullptr) {
			std::cout << "[Init error]: " << SDL_GetError() << std::endl;
			return;
		}

		ApplicationInfo appInfo{
			.name = std::move(name),
			.prefPath = prefpath,
			.gameLogger = std::make_unique<Logger>(appInfo.name, appInfo),
			.mainWindow = nullptr
		};

		SDL_free(prefpath);
		
		s_EngineLogger = std::make_unique<Logger>("Idio", appInfo);
		if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0) {
			s_EngineLogger->critical("Failed to init SDL: {}", SDL_GetError());
			crash();
		}

		appInfo.mainWindow = std::make_unique<Window>(wci);
		app.appInfo = &appInfo;
		app.init();
		bool open = true;
		while(open) {
			app.tick();

			poll_evts(app,
				[&](const QuitEvent& qe) -> bool { 
					open = false; 
					return true; 
				},
				
				[&](const WindowClosedEvent& wce) -> bool { 
					if(wce.id == appInfo.mainWindow->get_id()) {
						open = false;
						return true;
					}

					return false;
				}
			);
		}

		app.deinit();
		SDL_Quit();
	}

}