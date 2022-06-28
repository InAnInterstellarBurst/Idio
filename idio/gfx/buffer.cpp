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
		VmaAllocationCreateInfo aci{};
		check_vk(vmaCreateBuffer(c.get_allocator(), &bci, &aci, 
			&m_buffer, &m_alloc, &m_allocInfo), "Failed to create buffer");
	}
	
	template<BufferType T>
	Buffer<T>::~Buffer()
	{
		vmaDestroyBuffer(m_context.get_allocator(), m_buffer, m_alloc);
	}
}