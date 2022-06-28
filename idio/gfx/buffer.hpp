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
	class Context;

	enum class BufferType
	{
		Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
	};

	template<BufferType T>
	class Buffer
	{
	public:
		Buffer(const Context& c, vk::DeviceSize sz, bool staging = false);
		~Buffer();

		operator vk::Buffer() const noexcept { return m_buffer; }
	private:
		const Context& m_context;
		vk::DeviceSize m_size;
		vk::DeviceSize m_allocated = 0;

		void* m_data;
		bool m_staging;
		vk::Buffer m_buffer;
		VmaAllocation m_alloc;
		VmaAllocationInfo m_allocInfo;
	};
}