/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IDIO_GFX_BUFFER_H
#define IDIO_GFX_BUFFER_H

namespace idio
{
	class Context;

	enum class BufferType
	{
		Vertex = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		Index = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		Uniform = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT
	};

	enum class BufferUse
	{
		Staging,
		Gpu
	};

	template<BufferType T, BufferUse Use>
	class Buffer
	{
	public:
		Buffer(const Context &c, vk::DeviceSize sz);
		~Buffer();
		Buffer(const Buffer &o) = delete;
		Buffer &operator=(const Buffer &o) = delete;

		void write(auto *data, size_t sz, size_t offset)
		{
			if constexpr(Use == BufferUse::Staging) {
				uint8_t *dst = static_cast<uint8_t *>(m_mappedData) + offset;
				std::memcpy(dst, data, sz);
			}
		}

		void copy_from(vk::CommandBuffer cmd, Buffer<T, BufferUse::Staging> &o, size_t sz, size_t srcOffset, size_t dstOffset);

		operator vk::Buffer() const { return m_buffer; }

		static void bind(vk::CommandBuffer cmd, const std::vector<std::shared_ptr<Buffer<T, Use>>> &bfrs,
			const std::vector<vk::DeviceSize> &offsets);
	private:
		const Context &m_context;
		vk::DeviceSize m_size;
		vk::DeviceSize m_allocated = 0;

		void *m_mappedData = nullptr;
		vk::Buffer m_buffer;
		VmaAllocation m_alloc;
		VmaAllocationInfo m_allocInfo;
	};

	template<BufferUse Use>
	using VertexBuffer = Buffer<BufferType::Vertex, Use>;
	template<BufferUse Use>
	using IndexBuffer = Buffer<BufferType::Index, Use>;
	using UniformBuffer = Buffer<BufferType::Uniform, BufferUse::Gpu>;
}

#endif
