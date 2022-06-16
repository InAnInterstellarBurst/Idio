/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once
#include "event.hpp"

namespace Idio
{
	extern void main(const std::span<char*>& args);

	struct ApplicationInfo 
	{
		std::string name;
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
		const ApplicationInfo appInfo = {
			.name = std::move(name)
		};

		bool open = true;
		app.appInfo = &appInfo;
		app.init();
		while(open) {
			Event evt = poll_evts();
			bool handled = evt_handler(evt, 
				[&](const QuitEvent& qe) -> bool { open = false; return true; },
				[](const WindowClosedEvent& wce) -> bool { return false; }
			);

			if(!handled) {
				app.event_proc(evt);
			}
			
			app.tick();
		}

		app.deinit();
	}
}