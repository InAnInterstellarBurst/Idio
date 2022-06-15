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
	class Event {};

	using CommandArgs = std::vector<char*>;
	extern void main(const CommandArgs& args);

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

		app.appInfo = &appInfo;
		app.init();
		app.deinit();
	}
}