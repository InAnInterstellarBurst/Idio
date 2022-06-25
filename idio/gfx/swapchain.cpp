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

		m_imageAvailSems.resize(s_MaxFramesProcessing);
		vk::SemaphoreCreateInfo sci{};
		for(uint32_t i = 0; i < s_MaxFramesProcessing; i++) {
			m_imageAvailSems[i] = check_vk(c.get_device().createSemaphore(sci), "Failed to create image available semaphore");
		}

		create();
	}

	Swapchain::~Swapchain()
	{
		for(uint32_t i = 0; i < s_MaxFramesProcessing; i++) {
			m_context.get_device().destroySemaphore(m_imageAvailSems[i]);
		}

		for(auto iv : m_swapchainImageViews) {
			m_context.get_device().destroyImageView(iv);
		}
		m_context.get_device().destroySwapchainKHR(m_swapchain);
		m_context.get_instance().destroySurfaceKHR(m_surface);
	}

	void Swapchain::recreate()
	{
		check_vk(m_context.get_device().waitIdle(), "FUCK");
		for(auto iv : m_swapchainImageViews) {
			m_context.get_device().destroyImageView(iv);
		}
		create();
	}

	bool Swapchain::next()
	{
		constexpr uint64_t intmax = std::numeric_limits<uint64_t>::max();
		vk::Fence currentfence = m_context.get_gfx_queue_fences()[m_currentFrame];
		check_vk(m_context.get_device().waitForFences({ currentfence }, true, intmax),
			"Got impatient");

		auto imgres = m_context.get_device().acquireNextImageKHR(m_swapchain, intmax, m_imageAvailSems[m_currentFrame]);
		if(imgres.result == vk::Result::eSuboptimalKHR 
			|| imgres.result == vk::Result::eErrorOutOfDateKHR) {
			
			recreate();
			return false;
		} else if(imgres.result != vk::Result::eSuccess) {
			s_EngineLogger->critical("Failed to render frame");
			crash();
		}

		m_context.get_device().resetFences({ currentfence });
		m_imageIndex = imgres.value;
		return true;
	}

	void Swapchain::create()
	{
		const vk::PhysicalDevice pdev = m_context.get_physdev().handle;

		auto surfformats = pdev.getSurfaceFormatsKHR(m_surface).value;
		auto surfit = std::find_if(surfformats.begin(), surfformats.end(), 
			[](vk::SurfaceFormatKHR fmt) -> bool {
				return (fmt.format ==  vk::Format::eB8G8R8Srgb 
					&& fmt.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
			}
		);

		if(surfit != surfformats.end()) {
			m_format = *surfit;
		} else {
			m_format = surfformats[0];
		}

		auto presentmodes = pdev.getSurfacePresentModesKHR(m_surface).value;
		m_pmode = vk::PresentModeKHR::eFifo;
		if(m_window.is_vsync()) {
			auto pif = std::find(presentmodes.begin(), presentmodes.end(), vk::PresentModeKHR::eMailbox);
			if(pif != presentmodes.end()) {
				m_pmode = *pif;
			} else {
				s_EngineLogger->warning("VSync was requested but not supported, weird?");
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

	void Swapchain::present(std::vector<Swapchain*>& scs)
	{
		const Context& c = scs[0]->m_context; // God forgive me
		std::vector<vk::SwapchainKHR> swaps(scs.size());
		std::vector<uint32_t> imgidxs(scs.size());
		std::vector<vk::Semaphore> waitSems(scs.size());
		for(size_t i = 0; i < scs.size(); i++) {
			const auto sc = scs[i];
			swaps[i] = sc->m_swapchain;
			imgidxs[i] = sc->get_current_image_index();
			waitSems[i] = c.get_gfx_queue_finish_sems()[sc->get_current_frame_index()];
		}

		vk::PresentInfoKHR pi{};
		pi.waitSemaphoreCount   = static_cast<uint32_t>(waitSems.size());
		pi.pWaitSemaphores      = waitSems.data();
		pi.swapchainCount       = static_cast<uint32_t>(swaps.size());
		pi.pSwapchains          = swaps.data();
		pi.pImageIndices        = imgidxs.data();
		vk::Result pres = c.get_gfx_queue().presentKHR(pi); // Let's just hope the resize signal propogates 🙃
		if(pres != vk::Result::eSuccess 
			&& pres != vk::Result::eSuboptimalKHR && pres != vk::Result::eErrorOutOfDateKHR) {
			
			s_EngineLogger->critical("Failed to present");
			crash();
		}

		for(auto sc : scs) {
			sc->m_currentFrame = (sc->m_currentFrame + 1) % s_MaxFramesProcessing;
		}
	}
}