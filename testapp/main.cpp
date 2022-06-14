/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <idio/idio.hpp>
#include <iostream>

class App
{
public:
	void init() {}
	void tick() {}
	void deinit() {}
	void event_proc(const Idio::Event& e) {}

	const Idio::ApplicationInfo* appInfo = nullptr;
};

void Idio::entry_point(const Idio::CommandArgs& args)
{
	for(char* arg : args) {
		std::cout << arg << std::endl;
	}

	App app;
	Idio::run(app, std::string("Hello"));
}