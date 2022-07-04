/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include "context.hpp"

#include <SDL_vulkan.h>

#include "vkutl.hpp"
#include "core/app.hpp"
#include "swapchain.hpp"

#if ID_DEBUG
static VKAPI_ATTR VkBool32 debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT *data,
	void *ud)
{
	auto typestr = vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageType));
	switch(messageSeverity) {
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		idio::s_EngineLogger->error("{}: {}", typestr, data->pMessage);
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		idio::s_EngineLogger->warn("{}: {}", typestr, data->pMessage);
		break;
	default:
		idio::s_EngineLogger->trace("{}: {}", typestr, data->pMessage);
		break;
	}

	return VK_TRUE;
}
#endif


namespace idio
{
	Context::Context(const Version &v, const std::string &appname, const Window &w)
	{
		std::vector<const char *> vlayers;

		// Instance
		{
			vk::ApplicationInfo appInfo {};
			appInfo.apiVersion = VK_API_VERSION_1_3;
			appInfo.applicationVersion = VK_MAKE_VERSION(v.major, v.minor, v.patch);
			appInfo.pApplicationName = appname.c_str();
			appInfo.engineVersion = VK_MAKE_VERSION(k_EngineVersion.major, k_EngineVersion.minor, k_EngineVersion.patch);
			appInfo.pEngineName = "Idio";

			uint32_t extCount = 0;
			SDL_Vulkan_GetInstanceExtensions(w, &extCount, nullptr);
			std::vector<const char *> exts(extCount);
			SDL_Vulkan_GetInstanceExtensions(w, &extCount, exts.data());

			vk::InstanceCreateInfo ci {};
#if ID_DEBUG
			exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			vlayers.push_back("VK_LAYER_KHRONOS_validation");

			using enum vk::DebugUtilsMessageTypeFlagBitsEXT;
			using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
			vk::DebugUtilsMessengerCreateInfoEXT dci {};
			dci.pfnUserCallback = debug_callback;
			dci.messageSeverity = eError | eWarning | eInfo;
			dci.messageType = eGeneral | eValidation | ePerformance;

			ci.pNext = &dci;
#endif

			ci.pApplicationInfo = &appInfo;
			ci.enabledExtensionCount = static_cast<uint32_t>(exts.size());
			ci.ppEnabledExtensionNames = exts.data();
			ci.enabledLayerCount = static_cast<uint32_t>(vlayers.size());
			ci.ppEnabledLayerNames = vlayers.data();
			m_instance = check_vk(vk::createInstance(ci), "Failed to create instance");
			m_dispatchLoader = std::make_unique<vk::DispatchLoaderDynamic>(m_instance, vkGetInstanceProcAddr);
#if ID_DEBUG
			m_dbgmsgr = check_vk(m_instance.createDebugUtilsMessengerEXT(dci, nullptr, *m_dispatchLoader),
				"Failed to create debug msg");
#endif
		}

		// Device
		{
			auto rawpdevs = m_instance.enumeratePhysicalDevices().value;
			std::vector<PhysicalDevice> pdevs(rawpdevs.size());
			std::copy(rawpdevs.begin(), rawpdevs.end(), pdevs.begin());
			auto devit = std::max_element(pdevs.begin(), pdevs.end());
			if(devit == pdevs.end() || devit->gfxQueueFamilyIdx == std::numeric_limits<uint32_t>::max()) {
				s_EngineLogger->critical("No suitable graphics devices found.");
				Application::crash();
			}

			m_pdev = *devit;
			s_EngineLogger->info("Selected GPU {}", m_pdev.props.deviceName);

			constexpr float prior = 1.0f;
			vk::DeviceQueueCreateInfo qci {};
			qci.queueCount = 1;
			qci.pQueuePriorities = &prior;
			qci.queueFamilyIndex = m_pdev.gfxQueueFamilyIdx;

			const std::vector<const char *> exts { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
			vk::DeviceCreateInfo ci {};
			ci.queueCreateInfoCount = 1;
			ci.pQueueCreateInfos = &qci;
			ci.enabledLayerCount = static_cast<uint32_t>(vlayers.size());
			ci.ppEnabledLayerNames = vlayers.data();
			ci.enabledExtensionCount = static_cast<uint32_t>(exts.size());
			ci.ppEnabledExtensionNames = exts.data();
			m_device = check_vk(m_pdev.handle.createDevice(ci), "Failed to create device");
			m_gfxQueue = m_device.getQueue(m_pdev.gfxQueueFamilyIdx, 0);
		}

		vk::SemaphoreCreateInfo sci {};
		vk::FenceCreateInfo fci {};
		fci.flags = vk::FenceCreateFlagBits::eSignaled;
		for(uint32_t i = 0; i < s_MaxFramesProcessing; i++) {
			m_gfxQueueFences[i] = check_vk(m_device.createFence(fci), "Failed to create render finished fence");
			m_gfxFinishSems[i] = check_vk(m_device.createSemaphore(sci), "Failed to create gfx finish sem");
		}

		VmaAllocatorCreateInfo aci {};
		aci.device = m_device;
		aci.instance = m_instance;
		aci.physicalDevice = m_pdev.handle;
		check_vk(vmaCreateAllocator(&aci, &m_alloc), "Failed to create bfr alloc");
	}

	Context::~Context()
	{
		for(uint32_t i = 0; i < s_MaxFramesProcessing; i++) {
			m_device.destroyFence(m_gfxQueueFences[i]);
			m_device.destroySemaphore(m_gfxFinishSems[i]);
		}

		vmaDestroyAllocator(m_alloc);
		m_device.destroy();
#if ID_DEBUG
		m_instance.destroyDebugUtilsMessengerEXT(m_dbgmsgr, nullptr, *m_dispatchLoader);
#endif
		m_instance.destroy();
	}

	void Context::begin_cmd(vk::CommandBuffer buf) const
	{
		vk::CommandBufferBeginInfo bi {};
		check_vk(buf.begin(bi), "Failed to begin cmd buf");
	}

	void Context::end_cmd(vk::CommandBuffer buf) const
	{
		check_vk(buf.end(), "Failed to record cmd buf");
	}

	void Context::draw_cmd(vk::CommandBuffer buf, uint32_t vertCount) const
	{
		buf.draw(vertCount, 1, 0, 0);
	}

	void Context::submit_gfx_queue(const std::vector<vk::CommandBuffer> &cbufs, vk::Fence fence)
	{
		vk::SubmitInfo si {};
		si.commandBufferCount = static_cast<uint32_t>(cbufs.size());
		si.pCommandBuffers = cbufs.data();
		check_vk(m_gfxQueue.submit(si, fence), "Failed to submit gfx");
	}

	void Context::submit_gfx_queue(const Swapchain &sc, const std::vector<vk::CommandBuffer> &cbufs)
	{
		auto fi = sc.get_current_frame_index();
		const vk::Semaphore sigs[] = { m_gfxFinishSems[fi] };
		const vk::Semaphore imgav[] = { sc.get_current_image_avail_sem() };
		const vk::PipelineStageFlags waitstage[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

		vk::SubmitInfo si {};
		si.waitSemaphoreCount = 1;
		si.pWaitSemaphores = imgav;
		si.pWaitDstStageMask = waitstage;
		si.commandBufferCount = static_cast<uint32_t>(cbufs.size());
		si.pCommandBuffers = cbufs.data();
		si.signalSemaphoreCount = 1;
		si.pSignalSemaphores = sigs;
		check_vk(m_gfxQueue.submit(si, m_gfxQueueFences[fi]), "Failed to submit gfx");
	}


	CommandPool::CommandPool(const Context &c, bool transient) :
		m_dev(c.get_device())
	{
		vk::CommandPoolCreateInfo ci {};
		ci.queueFamilyIndex = c.get_physdev().gfxQueueFamilyIdx;
		ci.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
		if(transient) {
			ci.flags |= vk::CommandPoolCreateFlagBits::eTransient;
		}

		m_handle = check_vk(m_dev.createCommandPool(ci), "Failed to create gfx pool");
	}

	CommandPool::~CommandPool()
	{
		m_dev.destroyCommandPool(m_handle);
	}

	void CommandPool::reset()
	{
		m_dev.resetCommandPool(m_handle);
	}

	std::vector<vk::CommandBuffer> CommandPool::get_buffers(uint32_t count, bool secondary)
	{
		vk::CommandBufferAllocateInfo ai {};
		ai.commandPool = m_handle;
		ai.commandBufferCount = count;
		ai.level = secondary ? vk::CommandBufferLevel::eSecondary : vk::CommandBufferLevel::ePrimary;
		return check_vk(m_dev.allocateCommandBuffers(ai), "Failed to alloc command buffers");
	}
}
