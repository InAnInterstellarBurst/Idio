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
	struct ApplicationInfo;

	struct PhysicalDevice
	{
		vk::PhysicalDevice handle = nullptr;
		vk::PhysicalDeviceProperties props{};
		vk::PhysicalDeviceFeatures supportedFeatures{};
		uint32_t gfxQueueFamilyIdx = UINT32_MAX;

		PhysicalDevice() = default;
		PhysicalDevice(vk::PhysicalDevice pdev) : handle(pdev),
			props(handle.getProperties()), supportedFeatures(handle.getFeatures())
		{
			auto qfprops = handle.getQueueFamilyProperties();
			uint32_t i = 0;
			for(const auto& qfp : qfprops) {
				if(qfp.queueCount == 0) {
					i++;
					continue;
				}

				if(qfp.queueFlags & vk::QueueFlagBits::eGraphics) {
					gfxQueueFamilyIdx = i;
				}

				i++;
			}
		}

		bool operator<(const PhysicalDevice& other)
		{
			if(gfxQueueFamilyIdx == UINT32_MAX 
				&& other.gfxQueueFamilyIdx != UINT32_MAX) {
				
				return true;
			}

			if(other.props.deviceType == vk::PhysicalDeviceType::eCpu) {
				return false;
			}

			return (other.props.deviceType == vk::PhysicalDeviceType::eDiscreteGpu);
		}
	};

	class Context
	{
	public:
		Context(const ApplicationInfo& ai);
		~Context();

		vk::Instance get_instance() const { return m_instance; }
	private:
		vk::Instance m_instance;
		std::unique_ptr<vk::DispatchLoaderDynamic> m_dispatchLoader;
		PhysicalDevice m_pdev;
		vk::Device m_device;
		vk::Queue m_gfxQueue;

#if ID_DEBUG
		vk::DebugUtilsMessengerEXT m_dbgmsgr;
#endif
	};
}