/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "idio.hpp"
#include "context.hpp"
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
	Context::Context(const ApplicationInfo& ai)
	{
		// Instance
		{
			vk::ApplicationInfo appInfo{};
			appInfo.apiVersion           = VK_API_VERSION_1_3;
			appInfo.applicationVersion   = ai.version.as_vk_ver();
			appInfo.pApplicationName     = ai.name.c_str();
			appInfo.engineVersion        = s_EngineVersion.as_vk_ver();
			appInfo.pEngineName          = "Idio";

			std::vector<const char*> vlayers;

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
	}

	Context::~Context()
	{
#if ID_DEBUG
		m_instance.destroyDebugUtilsMessengerEXT(m_dbgmsgr, nullptr, *m_dispatchLoader);
#endif
		m_instance.destroy();
	}
}