/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include "swapchain.hpp"
#include "vkutl.hpp"
#include "context.hpp"
#include "core/window.hpp"
#include <SDL_vulkan.h>

namespace Idio
{
	Swapchain::Swapchain(const Context& c, const Window& w) : m_context(c)
	{
		VkSurfaceKHR rsurf;
		if(!SDL_Vulkan_CreateSurface(w, c.get_instance(), &rsurf)) {
			s_EngineLogger->critical("Failed to create surface: {}", SDL_GetError());
			crash();
		}

		m_surface = rsurf;
		create();
	}

	Swapchain::~Swapchain()
	{
		m_context.get_instance().destroySurfaceKHR(m_surface);
	}

	void Swapchain::recreate()
	{
		//cleanup
		create();
	}

	void Swapchain::create()
	{

	}
}