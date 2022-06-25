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
	class Window;
	class Pipeline;
	class Swapchain;
	struct Version;

	constexpr const uint32_t s_MaxFramesProcessing = 3;

	struct PhysicalDevice
	{
		vk::PhysicalDevice handle = nullptr;
		vk::PhysicalDeviceProperties props{};
		vk::PhysicalDeviceFeatures supportedFeatures{};
		uint32_t gfxQueueFamilyIdx = std::numeric_limits<uint32_t>::max();

		PhysicalDevice() = default;
		PhysicalDevice(vk::PhysicalDevice pdev) : handle(pdev),
			props(handle.getProperties()), supportedFeatures(handle.getFeatures()) // Implicit on purpose
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

		constexpr bool operator<(const PhysicalDevice& other)
		{
			constexpr auto uintmax = std::numeric_limits<uint32_t>::max();
			if(gfxQueueFamilyIdx == uintmax 
				&& other.gfxQueueFamilyIdx != uintmax) {
				
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
		Context(const Version& v, const std::string& appname, Window& w);
		~Context();

		void begin_cmd(vk::CommandBuffer buf);
		void end_cmd(vk::CommandBuffer buf);
		void draw_cmd(vk::CommandBuffer buf, uint32_t vertCount);
		
		void submit_gfx_queue(const Swapchain& sc, const std::vector<vk::CommandBuffer>& cbufs);

		vk::Instance get_instance() const { return m_instance; }
		vk::Device get_device() const { return m_device; }
		PhysicalDevice get_physdev() const { return m_pdev; }
		
		vk::Queue get_gfx_queue() const { return m_gfxQueue; }
		auto get_gfx_queue_finish_sems() const { return m_gfxFinishSems; }
		auto get_gfx_queue_fences() const { return m_gfxQueueFences; }
	private:
		vk::Instance m_instance;
		std::unique_ptr<vk::DispatchLoaderDynamic> m_dispatchLoader;
		PhysicalDevice m_pdev;
		vk::Device m_device;
		vk::Queue m_gfxQueue;
		std::array<vk::Fence, s_MaxFramesProcessing> m_gfxQueueFences;
		std::array<vk::Semaphore, s_MaxFramesProcessing> m_gfxFinishSems;

#if ID_DEBUG
		vk::DebugUtilsMessengerEXT m_dbgmsgr;
#endif
	};

	class CommandPool
	{
	public:
		explicit CommandPool(const Context& c, bool transient = false);
		~CommandPool();

		void reset();
		std::vector<vk::CommandBuffer> get_buffers(uint32_t count, bool secondary = false);
	private:
		vk::Device m_dev;
		vk::CommandPool m_handle;
	};
}