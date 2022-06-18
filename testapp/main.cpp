/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <idio/idio.hpp>

class App
{
public:
	void init() 
	{
	}
	
	void tick()
	{
	}

	void deinit() 
	{
	}

	void event_proc(const Idio::Event& e) 
	{
		Idio::evt_handler(e, 
			[](const Idio::QuitEvent& qe) -> bool { return true; },
			[](const Idio::WindowClosedEvent& ce) -> bool { return true; }
		);
	}

	const Idio::ApplicationInfo* appInfo = nullptr;
};

void Idio::main(const std::span<char*>& args)
{
	App app;
	Idio::run(app, {}, Idio::Version{0, 0, 1}, std::string("Hello"));
}