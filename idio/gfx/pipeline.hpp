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
	class Swapchain;

	std::optional<std::vector<uint32_t>> load_shader_from_disk(const std::string& pth);

	struct VertexLayout
	{
		uint32_t stride = 0;
		uint32_t binding = 0;
		bool instance = false;
	};

	enum class AttribFormat
	{
		Float = VK_FORMAT_R32_SFLOAT,
		Vec2 = VK_FORMAT_R32G32_SFLOAT,
		Vec3 = VK_FORMAT_R32G32B32_SFLOAT,
		Vec4 = VK_FORMAT_R32G32B32A32_SFLOAT
	};

	struct AttributeDescription
	{
		uint32_t offset = 0;
		uint32_t binding = 0;
		uint32_t location = 0;
		AttribFormat format = AttribFormat::Float;
	};

	struct PipelineCreateInfo
	{
		std::string cacheName;
		std::vector<uint32_t> vertexShaderCode;
		std::vector<uint32_t> fragmentShaderCode;

		bool blendAlpha = false;
		bool primitiveRestart = false;
		vk::PolygonMode polyMode = vk::PolygonMode::eFill;
		vk::PrimitiveTopology topology = vk::PrimitiveTopology::eTriangleList;
		
		std::vector<VertexLayout> vertexLayouts;
		std::vector<AttributeDescription> attributeDescs;
	};

	class Pipeline
	{
	public:
		Pipeline(vk::Device dev, const Swapchain& sc, const PipelineCreateInfo& pci);
		~Pipeline();

		void reset();
		void bind_cmd(vk::CommandBuffer buf) const;
		void unbind_cmd(vk::CommandBuffer buf) const;
	private:
		vk::Device m_dev;
		const Swapchain& m_swapchain;

		vk::Pipeline m_handle;
		vk::PipelineLayout m_layout;
		vk::RenderPass m_rpass;
		std::vector<vk::Framebuffer> m_framebufs;

		void create_renderpass();
		void create_framebuffers();
	};
}
