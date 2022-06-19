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
	constexpr std::pair<int32_t, int32_t> s_WindowPosCentred { SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED };
	constexpr std::pair<int32_t, int32_t> s_WindowPosDefault { SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED };
	
	class Context;
	class Swapchain;

	enum FullscreenState
	{
		Normal,
		Borderless,
		Exclusive
	};

	struct WindowCreateInfo
	{
		bool vsync = false;
		bool borderless = false;
		bool resizeable = false;
		int32_t width = 1280;
		int32_t height = 720;
		const char* title = "Idiot";

		FullscreenState state = FullscreenState::Normal;
		std::pair<int32_t, int32_t> position = s_WindowPosDefault;
	};

	class Window
	{
	public:
		Window(const WindowCreateInfo& wci);
		~Window();

		void post_close_evt();
		void set_fullscreen_state(FullscreenState s);
		void create_swapchain(const Context& c);

		uint32_t get_id() const { return m_id; }
		Swapchain& get_swapchain() const { return *m_swapchain; }

		bool operator==(const Window& o) const { return m_id == o.m_id; }
		operator SDL_Window*() const { return m_handle; }
	private:
		uint32_t m_id = 0;
		bool m_vsync = false;
		SDL_Window* m_handle = nullptr;
		Swapchain* m_swapchain = nullptr;
		FullscreenState m_fullscrState = FullscreenState::Normal;
	};
}