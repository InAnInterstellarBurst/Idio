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

		vk::Extent2D get_extent() const { return m_extent; }
		vk::Format get_format() const { return m_format.format; }
		auto get_image_views() const { return m_swapchainImageViews; }
	private:
		const Window& m_window;
		const Context& m_context;

		vk::SurfaceKHR m_surface;
		vk::SwapchainKHR m_swapchain = nullptr;
		vk::Extent2D m_extent;
		vk::PresentModeKHR m_pmode;
		vk::SurfaceFormatKHR m_format;

		std::vector<vk::Image> m_swapchainImages;
		std::vector<vk::ImageView> m_swapchainImageViews;

		void create();
	};
}