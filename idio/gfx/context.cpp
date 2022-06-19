/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include "context.hpp"
#include "vkutl.hpp"
#include "core/app.hpp"
#include <SDL_vulkan.h>

#if ID_DEBUG
	static VKAPI_ATTR VkBool32 debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* data,
		void* ud)
	{
		auto typestr = vk::to_string(static_cast<vk::DebugUtilsMessageTypeFlagsEXT>(messageType));
		switch(messageSeverity) {
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			Idio::s_EngineLogger->error("{}: {}", typestr, data->pMessage);
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			Idio::s_EngineLogger->warning("{}: {}", typestr, data->pMessage);
			break;
		default:
			Idio::s_EngineLogger->trace("{}: {}", typestr, data->pMessage);
			break;
		}

		return VK_TRUE;
	}
#endif


namespace Idio
{
	constexpr Version s_EngineVersion { 0, 0, 0 };

	Context::Context(const ApplicationInfo& ai)
	{
		std::vector<const char*> vlayers;

		// Instance
		{
			vk::ApplicationInfo appInfo{};
			appInfo.apiVersion           = VK_API_VERSION_1_3;
			appInfo.applicationVersion   = ai.version.as_vk_ver();
			appInfo.pApplicationName     = ai.name.c_str();
			appInfo.engineVersion        = s_EngineVersion.as_vk_ver();
			appInfo.pEngineName          = "Idio";

			uint32_t extCount = 0;
			SDL_Vulkan_GetInstanceExtensions(*ai.mainWindow, &extCount, nullptr);
			std::vector<const char*> exts(extCount + 1);
			SDL_Vulkan_GetInstanceExtensions(*ai.mainWindow, &extCount, exts.data());

			vk::InstanceCreateInfo ci{};
#if ID_DEBUG
			exts[extCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
			vlayers.push_back("VK_LAYER_KHRONOS_validation");

			using enum vk::DebugUtilsMessageTypeFlagBitsEXT;
			using enum vk::DebugUtilsMessageSeverityFlagBitsEXT;
			vk::DebugUtilsMessengerCreateInfoEXT dci{};
			dci.pfnUserCallback    = debug_callback;
			dci.messageSeverity    = eError | eWarning | eInfo;
			dci.messageType        = eGeneral | eValidation | ePerformance;

			ci.pNext = &dci;
#endif

			ci.pApplicationInfo        = &appInfo;
			ci.enabledExtensionCount   = static_cast<uint32_t>(exts.size());
			ci.ppEnabledExtensionNames = exts.data();
			ci.enabledLayerCount       = static_cast<uint32_t>(vlayers.size());
			ci.ppEnabledLayerNames     = vlayers.data();
			m_instance = check_vk(vk::createInstance(ci));
			m_dispatchLoader = std::make_unique<vk::DispatchLoaderDynamic>(m_instance, vkGetInstanceProcAddr);
#if ID_DEBUG
			m_dbgmsgr = check_vk(m_instance.createDebugUtilsMessengerEXT(dci, nullptr, *m_dispatchLoader));
#endif
		}

		// Device
		{
			auto rawpdevs = m_instance.enumeratePhysicalDevices().value;
			std::vector<PhysicalDevice> pdevs(rawpdevs.size());
			std::copy(rawpdevs.begin(),	rawpdevs.end(), pdevs.begin());
			auto devit = std::max_element(pdevs.begin(), pdevs.end());
			if(devit == pdevs.end() || devit->gfxQueueFamilyIdx == UINT32_MAX) {
				s_EngineLogger->critical("No suitable graphics devices found.");
				crash();
			}

			m_pdev = *devit;
			s_EngineLogger->info("Selected GPU {}", m_pdev.props.deviceName);

			constexpr float prior = 1.0f;
			vk::DeviceQueueCreateInfo qci{};
			qci.queueCount         = 1;
			qci.pQueuePriorities   = &prior;
			qci.queueFamilyIndex   = m_pdev.gfxQueueFamilyIdx;

			const std::vector<const char*> exts{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
			vk::DeviceCreateInfo ci{};
			ci.queueCreateInfoCount    = 1;
			ci.pQueueCreateInfos       = &qci;
			ci.enabledLayerCount       = static_cast<uint32_t>(vlayers.size());
			ci.ppEnabledLayerNames     = vlayers.data();
			ci.enabledExtensionCount   = static_cast<uint32_t>(exts.size());
			ci.ppEnabledExtensionNames = exts.data();
			m_device = check_vk(m_pdev.handle.createDevice(ci));
			m_gfxQueue = m_device.getQueue(m_pdev.gfxQueueFamilyIdx, 0);
		}
	}

	Context::~Context()
	{
		m_device.destroy();
#if ID_DEBUG
		m_instance.destroyDebugUtilsMessengerEXT(m_dbgmsgr, nullptr, *m_dispatchLoader);
#endif
		m_instance.destroy();
	}
}