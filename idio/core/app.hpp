/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "core/logger.hpp"
#include "core/event.hpp"
#include "core/window.hpp"

namespace Idio
{
	extern void main(const std::span<char*>& args);
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
		Window* mainWindow;
		Context* context;
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


	// In a vain attempt to cut compile times I'll move context stuff out
	namespace Internal
	{
		ApplicationInfo init_engine(const WindowCreateInfo& wci, Version v, std::string name);
		void deinit_engine(ApplicationInfo& app);
	}

	template<Application App>
	void run(App& app, const WindowCreateInfo& wci, Version v, std::string name)
	{
		auto appInfo = Internal::init_engine(wci, std::move(v), std::move(name));
		app.appInfo = &appInfo;
		app.init();
		bool open = true;
		bool minimised = false;
		while(open) {
			app.tick();

			poll_evts(app,
				[&](const QuitEvent& qe) -> bool { 
					open = false; 
					return true; 
				},

				[&](const WindowMinimiseEvent& me) -> bool {
					if(me.id == appInfo.mainWindow->get_id()) {
						minimised = me.minimised;
						return true;
					}

					return false;
				},
				[&](const WindowClosedEvent& wce) -> bool { 
					if(wce.id == appInfo.mainWindow->get_id()) {
						open = false;
						return true;
					}

					return false;
				},
				[&](const WindowResizeEvent& wre) -> bool { 
					if(wre.id == appInfo.mainWindow->get_id()) {
						appInfo.mainWindow->create_swapchain(*appInfo.context);
						return true;
					}

					return false;
				}
			);
		}

		app.deinit();
		Internal::deinit_engine(appInfo);
		SDL_Quit();
	}

}