/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

namespace Idio
{
	class Window;
	class Context;
	
	class Swapchain
	{
	public:
		Swapchain(const Context& c, const Window& w);
		~Swapchain();

		void recreate();
	private:
		const Context& m_context;
		vk::SurfaceKHR m_surface;
		vk::SwapchainKHR m_swapchain;

		void create();
	};
}