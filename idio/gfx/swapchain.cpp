/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include "swapchain.hpp"
#include <SDL_vulkan.h>
#include "vkutl.hpp"
#include "context.hpp"
#include "core/window.hpp"

namespace Idio
{
	Swapchain::Swapchain(const Context& c, const Window& w) :
		m_window(w), m_context(c)
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
		for(auto iv : m_swapchainImageViews) {
			m_context.get_device().destroyImageView(iv);
		}
		m_context.get_device().destroySwapchainKHR(m_swapchain);
		m_context.get_instance().destroySurfaceKHR(m_surface);
	}

	void Swapchain::recreate()
	{
		for(auto iv : m_swapchainImageViews) {
			m_context.get_device().destroyImageView(iv);
		}
		create();
	}

	void Swapchain::create()
	{
		const vk::PhysicalDevice pdev = m_context.get_physdev().handle;

		auto surfformats = pdev.getSurfaceFormatsKHR(m_surface).value;
		m_format = surfformats[0];
		for(auto fmt : surfformats) {
			if(fmt.format ==  vk::Format::eB8G8R8Srgb && fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				m_format = fmt;
				break;
			}
		}

		auto presentmodes = pdev.getSurfacePresentModesKHR(m_surface).value;
		m_pmode = vk::PresentModeKHR::eFifo;
		for(const auto& pmode : presentmodes) {
			if(!m_window.is_vsync() && pmode == vk::PresentModeKHR::eMailbox) {
				m_pmode = pmode;
				break;
			}
		}

		auto caps = pdev.getSurfaceCapabilitiesKHR(m_surface).value;
		uint32_t imageCount = caps.minImageCount + 1;
		if(caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			m_extent = caps.currentExtent;
		} else { // Supporting apple retna despite apple not supporting vulkan... good move :)
			int width, height;
			SDL_GetWindowSize(m_window, &width, &height);
			m_extent = vk::Extent2D {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			m_extent.width = std::clamp(m_extent.width, caps.minImageExtent.width, 
				caps.maxImageExtent.width);
			m_extent.height = std::clamp(m_extent.height, caps.minImageExtent.height, 
				caps.maxImageExtent.height);
		}

		vk::SwapchainCreateInfoKHR ci{};
		ci.surface			= m_surface;
		ci.minImageCount	= imageCount;
		ci.imageFormat		= m_format.format;
		ci.imageColorSpace	= m_format.colorSpace;
		ci.imageExtent		= m_extent;
		ci.imageArrayLayers = 1;
		ci.imageUsage		= vk::ImageUsageFlagBits::eColorAttachment;
		ci.imageSharingMode = vk::SharingMode::eExclusive;
		ci.preTransform		= caps.currentTransform;
		ci.compositeAlpha	= vk::CompositeAlphaFlagBitsKHR::eOpaque;
		ci.presentMode		= m_pmode;
		ci.clipped			= true;
		ci.oldSwapchain		= m_swapchain;
		m_swapchain = check_vk(m_context.get_device().createSwapchainKHR(ci), "Failed to create swapchain");
		if(ci.oldSwapchain) {
			m_context.get_device().destroySwapchainKHR(ci.oldSwapchain);
		}

		m_swapchainImages = m_context.get_device().getSwapchainImagesKHR(m_swapchain).value;
		m_swapchainImageViews.resize(imageCount);
		for(uint32_t i = 0; i < imageCount; i++) {
			vk::ImageViewCreateInfo ici {};
			ici.image							= m_swapchainImages[i];
			ici.format							= m_format.format;
			ici.viewType						= vk::ImageViewType::e2D;
			ici.components						= { vk::ComponentSwizzle::eIdentity };
			ici.subresourceRange.aspectMask		= vk::ImageAspectFlagBits::eColor;
			ici.subresourceRange.baseMipLevel   = 0;
			ici.subresourceRange.levelCount		= 1;
			ici.subresourceRange.baseArrayLayer = 0;
			ici.subresourceRange.layerCount		= 1;
			m_swapchainImageViews[i] = check_vk(m_context.get_device().createImageView(ici), 
				"Failed to create swapchain image views");
		}
	}
}