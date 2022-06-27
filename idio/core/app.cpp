/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include "app.hpp"
#include "gfx/context.hpp"

namespace Idio::Internal
{
	std::shared_ptr<ApplicationInfo> init_engine(const WindowCreateInfo& wci, Version v,
		std::string name)
	{
		char* prefpath = SDL_GetPrefPath("idio", name.c_str());
		if(prefpath == nullptr) {
			std::cout << "[Init error]: " << SDL_GetError() << std::endl;
			return nullptr;
		}

		auto appInfo = std::make_shared<ApplicationInfo>();
		appInfo->version = v;
		appInfo->name = std::move(name);
		appInfo->prefPath = prefpath;
		appInfo->gameLogger = std::make_unique<Logger>(appInfo->name, *appInfo);
		appInfo->context = nullptr;
		appInfo->mainWindow = nullptr;

		SDL_free(prefpath);
		
		s_EngineLogger = std::make_unique<Logger>("Idio", *appInfo);
		if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0) {
			s_EngineLogger->critical("Failed to init SDL: {}", SDL_GetError());
			crash();
		}

		appInfo->mainWindow = std::make_unique<Window>(wci);
		appInfo->context = std::make_unique<Context>(v, appInfo->name, *appInfo->mainWindow);
		appInfo->mainWindow->create_swapchain(*appInfo->context);
		return appInfo;
	}
}
