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
	const Idio::ApplicationInfo* appInfo;
	std::unique_ptr<Idio::Pipeline> pipeline;

	void init() 
	{
		auto vscode = Idio::load_shader_from_disk("./shaders/basic.vert.spv");
		auto fscode = Idio::load_shader_from_disk("./shaders/basic.frag.spv");
		if(!vscode || !fscode) {
			appInfo->gameLogger->critical("Oh dear one of the shaders are invalid");
			Idio::crash();
		}

		Idio::PipelineCreateInfo pci{};
		pci.vertexShaderCode = *vscode;
		pci.fragmentShaderCode = *fscode;

		pipeline = std::make_unique<Idio::Pipeline>(appInfo->context->get_device(), 
			appInfo->mainWindow->get_swapchain(), pci);
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
			[](const Idio::WindowClosedEvent& ce) -> bool { return true; },
			[](const Idio::WindowResizeEvent& re) -> bool { return true; },
			[](const Idio::WindowMinimiseEvent& me) -> bool { return true; }
		);
	}
};

void Idio::main(const std::span<char*>& args)
{
	Idio::run(App{}, Idio::WindowCreateInfo{ .resizeable = true }, Idio::Version{0, 0, 1}, std::string("Hello"));
}