/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include "window.hpp"
#include "gfx/context.hpp"
#include "gfx/swapchain.hpp"

namespace Idio
{
	Window::Window(const WindowCreateInfo& wci) : m_vsync(wci.vsync)
	{
		uint32_t winflags = SDL_WINDOW_VULKAN;
		if(wci.borderless) {
			winflags |= SDL_WINDOW_BORDERLESS;
		}
		
		if(wci.resizeable) {
			winflags |= SDL_WINDOW_RESIZABLE;
		}

		auto[x, y] = wci.position;
		m_handle = SDL_CreateWindow(wci.title, x, y, wci.width, wci.height, winflags);
		if(m_handle == nullptr) {
			s_EngineLogger->critical("Failed to create window: {}", SDL_GetError());
			crash();
		}

		m_id = SDL_GetWindowID(m_handle);
		set_fullscreen_state(wci.state);
	}

	Window::~Window()
	{
		delete m_swapchain;
		SDL_DestroyWindow(m_handle);
	}
	
	void Window::post_close_evt()
	{
		SDL_Event close{ .type = SDL_WINDOWEVENT };
		close.window.event = SDL_WINDOWEVENT_CLOSE;
		close.window.windowID = m_id;
		SDL_PushEvent(&close);
	}
	
	void Window::set_fullscreen_state(FullscreenState s)
	{
		if(s == m_fullscrState) {
			return;
		}

		uint32_t f = 0;
		if(s == FullscreenState::Borderless) {
			f = SDL_WINDOW_FULLSCREEN;
		} else if(s == FullscreenState::Exclusive) {
			f = SDL_WINDOW_FULLSCREEN_DESKTOP;
		}

		SDL_SetWindowFullscreen(m_handle, f);
		m_fullscrState = s;
	}

	void Window::create_swapchain(const Context& c)
	{
		if(m_swapchain == nullptr) {
			m_swapchain = new Swapchain(c, *this);
		} else {
			m_swapchain->recreate();
		}
	}

	bool Window::clear()
	{
		return m_swapchain->next();
	}

	void Window::present()
	{
		static std::vector<Swapchain*> ssv{ m_swapchain };
		Swapchain::present(ssv);
	}
}