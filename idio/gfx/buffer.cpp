/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include "buffer.hpp"
#include "context.hpp"

namespace Idio
{
	template<BufferType T>
	Buffer<T>::Buffer(const Context& c, vk::DeviceSize sz, bool staging = false)
		: m_context(c), m_size(sz), m_staging(staging)
	{
		vk::BufferCreateInfo bci{};
		bci.size          = sz;
		bci.sharingMode   = vk::SharingMode::eExclusive; // We don't support transfer queues yet
		bci.usage         = static_cast<vk::BufferUsageFlagBits>(T);
		if(staging) {
			bci.usage |= vk::BufferUsageFlagBits::eTransferSrc;
		} else {
			bci.usage |= vk::BufferUsageFlagBits::eTransferSrc;
		}

		VmaAllocationCreateInfo aci{};
		aci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		if constexpr(T == BufferType::Uniform) {
			aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		} else {
			aci.usage = staging ? VMA_MEMORY_USAGE_CPU_ONLY : VMA_MEMORY_USAGE_GPU_ONLY;
		}
		check_vk(vmaCreateBuffer(c.get_allocator(), &bci, &aci, &m_buffer, &m_alloc, &m_allocInfo), "Failed to create buffer");
	}
	
	template<BufferType T>
	Buffer<T>::~Buffer()
	{
		vmaDestroyBuffer(m_context.get_allocator(), m_buffer, m_alloc);
	}
}