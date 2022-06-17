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
		m_two = std::make_unique<Idio::Window>(Idio::WindowCreateInfo{ .width = 300, .height = 300, .title = "Woo" });
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
			[&](const Idio::WindowClosedEvent& ce) -> bool {
				if(ce.id == m_two->get_id()) {
					m_two.reset();
				}
				return true;
			}
		);
	}

	std::unique_ptr<Idio::Window> m_two = nullptr;
	const Idio::ApplicationInfo* appInfo = nullptr;
};

void Idio::main(const std::span<char*>& args)
{
	App app;
	Idio::run(app, {}, std::string("Hello"));
}