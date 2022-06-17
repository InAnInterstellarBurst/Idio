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
	class Context;
	
	struct Version
	{
		uint32_t major;
		uint32_t minor;
		uint32_t patch;
		
		uint32_t as_vk_ver() const { return VK_MAKE_VERSION(major, minor, patch); }
	};

	struct ApplicationInfo 
	{
		std::string name;
		Version version;

		std::string prefPath;
		std::unique_ptr<Logger> gameLogger;
		std::unique_ptr<Window> mainWindow;
		std::unique_ptr<Context> context;
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

	ApplicationInfo init_engine(const WindowCreateInfo& wci, Version v, std::string name);

	template<Application App>
	void run(App& app, const WindowCreateInfo& wci, Version v, std::string name)
	{
		auto appInfo = init_engine(wci, std::move(v), std::move(name));
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