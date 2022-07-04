/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include "buffer.hpp"

#include "vkutl.hpp"
#include "context.hpp"

namespace idio
{
	template<BufferType T, BufferUse Use>
	Buffer<T, Use>::Buffer(const Context &c, vk::DeviceSize sz) :
		m_context(c), m_size(sz)
	{
		vk::BufferCreateInfo bci {};
		bci.size = sz;
		bci.sharingMode = vk::SharingMode::eExclusive; // We don't support transfer queues yet
		bci.usage = static_cast<vk::BufferUsageFlagBits>(T);
		if constexpr(Use == BufferUse::Staging) {
			bci.usage |= vk::BufferUsageFlagBits::eTransferSrc;
		} else {
			bci.usage |= vk::BufferUsageFlagBits::eTransferDst;
		}

		VmaAllocationCreateInfo aci {};
		aci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
		if constexpr(T == BufferType::Uniform) {
			aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		} else if constexpr(Use == BufferUse::Staging) {
			aci.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		} else {
			aci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		}

		VkBuffer tmpbuf;
		auto rawbci = static_cast<VkBufferCreateInfo>(bci);
		check_vk(vmaCreateBuffer(c.get_allocator(), &rawbci, &aci, &tmpbuf, &m_alloc, &m_allocInfo), "Failed to create buffer");
		m_buffer = static_cast<vk::Buffer>(tmpbuf);
		if constexpr(Use == BufferUse::Staging) {
			m_mappedData = m_allocInfo.pMappedData;
		}
	}

	template<BufferType T, BufferUse Use>
	Buffer<T, Use>::~Buffer()
	{
		vmaDestroyBuffer(m_context.get_allocator(), m_buffer, m_alloc);
	}

	template<BufferType T, BufferUse Use>
	void Buffer<T, Use>::copy_from(vk::CommandBuffer cmd, Buffer<T, BufferUse::Staging> &o, size_t sz, size_t srcOffset, size_t dstOffset)
	{
		vk::BufferCopy cbi {};
		cbi.size = sz;
		cbi.srcOffset = srcOffset;
		cbi.dstOffset = dstOffset;
		cmd.copyBuffer(o, m_buffer, cbi);
	}

	template<>
	void VertexBuffer<BufferUse::Gpu>::bind(vk::CommandBuffer cmd, const std::vector<std::shared_ptr<VertexBuffer<BufferUse::Gpu>>> &bfrs,
		const std::vector<vk::DeviceSize> &offsets)
	{
		std::vector<vk::Buffer> hdls(bfrs.size());
		std::transform(bfrs.begin(), bfrs.end(), hdls.begin(), [](std::shared_ptr<VertexBuffer<BufferUse::Gpu>> v) { return v->m_buffer; });
		cmd.bindVertexBuffers(0, hdls, offsets);
	}

	template<>
	void IndexBuffer<BufferUse::Gpu>::bind(vk::CommandBuffer cmd, const std::vector<std::shared_ptr<IndexBuffer<BufferUse::Gpu>>> &bfrs,
		const std::vector<vk::DeviceSize> &offsets)
	{
		cmd.bindIndexBuffer(bfrs[0]->m_buffer, offsets[0], vk::IndexType::eUint32);
	}

	template class Buffer<BufferType::Vertex, BufferUse::Gpu>;
	template class Buffer<BufferType::Vertex, BufferUse::Staging>;
	template class Buffer<BufferType::Index, BufferUse::Gpu>;
	template class Buffer<BufferType::Index, BufferUse::Staging>;
	template class Buffer<BufferType::Uniform, BufferUse::Gpu>;
}