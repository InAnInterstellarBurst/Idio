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
#include "gfx/context.hpp"

namespace Idio
{
	extern void main(const std::span<char*>& args);
	
	struct Version
	{
		uint32_t major;
		uint32_t minor;
		uint32_t patch;
		
		constexpr uint32_t as_vk_ver() const noexcept { return VK_MAKE_VERSION(major, minor, patch); }
	};

	struct ApplicationInfo 
	{
		Version version;
		std::string name;
		std::string prefPath;

		std::unique_ptr<Logger> gameLogger;
		std::unique_ptr<Context> context;
		std::unique_ptr<Window> mainWindow;
	};

	template<class T>
	concept Application = requires(T t, const Event& e)
	{
		t.init();
		t.tick();
		t.event_proc(e);
		t.recreate_pipelines();
		requires std::is_default_constructible_v<T>;
		requires std::same_as<decltype(t.appInfo), const ApplicationInfo*>;
	};


	// In a vain attempt to cut compile times I'll move context stuff out
	namespace Internal
	{
		std::shared_ptr<ApplicationInfo> init_engine(const WindowCreateInfo& wci, Version v,
			std::string name);
	}

	template<Application App>
	void run(const WindowCreateInfo& wci, Version v, std::string name)
	{
		auto appInfo = Internal::init_engine(wci, v, std::move(name));
		App app;
		app.appInfo = appInfo.get();
		app.init();

		bool open = true;
		bool minimised = false;
		while(open) {
			if(!minimised) {
				if(!appInfo->mainWindow->clear()) {
					app.recreate_pipelines();
					continue;
				}

				app.tick();
			}

			poll_evts(app,
				[&](const QuitEvent& qe) -> bool {
					open = false;
					return true;
				},

				[&](const WindowMinimiseEvent& me) -> bool {
					if(me.id == appInfo->mainWindow->get_id()) {
						minimised = me.minimised;
						return true;
					}

					return false;
				},
				[&](const WindowClosedEvent& wce) -> bool {
					if(wce.id == appInfo->mainWindow->get_id()) {
						open = false;
						return true;
					}

					return false;
				},
				[&](const WindowResizeEvent& wre) -> bool {
					if(wre.id == appInfo->mainWindow->get_id()) {
						appInfo->mainWindow->create_swapchain(*appInfo->context);
						app.recreate_pipelines();
						return true;
					}

					return false;
				}
			);
		}
	}
}
