/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include "pipeline.hpp"
#include <fstream>
#include "swapchain.hpp"
#include "vkutl.hpp"

namespace Idio
{
	std::optional<std::vector<uint32_t>> load_shader_from_disk(const std::string& pth)
	{
		std::ifstream file(pth, std::ios::ate | std::ios::binary);
		if(!file) {
			s_EngineLogger->warning("Failed to load shader {}", pth);
			return {};
		}

		auto len = file.tellg();
		std::vector<uint32_t> data(static_cast<size_t>(len));
		file.seekg(0);
		file.read(reinterpret_cast<char*>(data.data()), len);
		file.close();
		return data;
	}


	Pipeline::Pipeline(vk::Device dev, const Swapchain& sc, 
		const PipelineCreateInfo& pci) : m_dev(dev), m_swapchain(sc)
	{
		vk::ShaderModuleCreateInfo sci{};
		sci.codeSize = pci.vertexShaderCode.size();
		sci.pCode = pci.vertexShaderCode.data();
		vk::ShaderModule vshader = check_vk(m_dev.createShaderModule(sci), "Failed to create vert shader");

		sci.codeSize = pci.fragmentShaderCode.size();
		sci.pCode = pci.fragmentShaderCode.data();
		vk::ShaderModule fshader = check_vk(m_dev.createShaderModule(sci), "Failed to create frag shder");

		vk::PipelineShaderStageCreateInfo vsci{};
		vsci.pName  = "main";
		vsci.module = vshader;
		vsci.stage  = vk::ShaderStageFlagBits::eVertex;

		vk::PipelineShaderStageCreateInfo fsci{};
		fsci.pName  = "main";
		fsci.module = fshader;
		fsci.stage  = vk::ShaderStageFlagBits::eFragment;
		vk::PipelineShaderStageCreateInfo stages[2] = { vsci, fsci };

		std::vector<vk::VertexInputBindingDescription> bindDescs(pci.vertexLayouts.size());
		std::transform(pci.vertexLayouts.begin(), pci.vertexLayouts.end(), bindDescs.begin(), 
			[](const VertexLayout& l) { 
				return vk::VertexInputBindingDescription{
					l.binding, l.stride,
					l.instance ? vk::VertexInputRate::eInstance : vk::VertexInputRate::eVertex
				};
			}
		);

		std::vector<vk::VertexInputAttributeDescription> attrdsc(pci.attributeDescs.size());
		std::transform(pci.attributeDescs.begin(), pci.attributeDescs.end(), attrdsc.begin(),
			[](const AttributeDescription& f) {
				return vk::VertexInputAttributeDescription{
					f.location, f.binding, static_cast<vk::Format>(f.format), f.offset
				};
			}
		);

		vk::PipelineVertexInputStateCreateInfo vci{};
		vci.vertexBindingDescriptionCount   = static_cast<uint32_t>(bindDescs.size());
		vci.pVertexBindingDescriptions      = bindDescs.data();
		vci.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrdsc.size());
		vci.pVertexAttributeDescriptions    = attrdsc.data();

		vk::PipelineInputAssemblyStateCreateInfo iaci{};
		iaci.topology = pci.topology;
		iaci.primitiveRestartEnable = pci.primitiveRestart;

		vk::Rect2D scis{};
		scis.extent = sc.get_extent();

		vk::Viewport vp{};
		vp.x        = 0;
		vp.y        = 0;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width    = static_cast<float>(sc.get_extent().width);
		vp.height   = static_cast<float>(sc.get_extent().height);

		vk::PipelineViewportStateCreateInfo vpci{};
		vpci.scissorCount  = 1;
		vpci.pScissors     = &scis;
		vpci.viewportCount = 1;
		vpci.pViewports    = &vp;

		vk::PipelineRasterizationStateCreateInfo rci{};
		rci.depthClampEnable        = false;
		rci.rasterizerDiscardEnable = false;
		rci.polygonMode             = pci.polyMode;
		rci.lineWidth               = 1.0f;
		rci.cullMode                = vk::CullModeFlagBits::eBack;
		rci.frontFace               = vk::FrontFace::eClockwise;
		rci.depthBiasEnable         = false;

		//TODO: Might be helpful... :)
		vk::PipelineMultisampleStateCreateInfo msci{};
		msci.sampleShadingEnable = false;

		//TODO: This is also cool and ideal :)
		vk::PipelineDepthStencilStateCreateInfo dci{};

		using enum vk::ColorComponentFlagBits;
		vk::PipelineColorBlendAttachmentState blendInfo{};
		blendInfo.colorWriteMask      = eR | eG | eB | eA;
		blendInfo.blendEnable         = pci.blendAlpha;
		blendInfo.srcColorBlendFactor = vk::BlendFactor::eSrcAlpha;
		blendInfo.dstColorBlendFactor = vk::BlendFactor::eOneMinusSrcAlpha;
		blendInfo.colorBlendOp        = vk::BlendOp::eAdd;
		blendInfo.srcAlphaBlendFactor = vk::BlendFactor::eOne;
		blendInfo.dstAlphaBlendFactor = vk::BlendFactor::eZero;
		blendInfo.alphaBlendOp        = vk::BlendOp::eAdd;

		vk::PipelineColorBlendStateCreateInfo cbci{};
		cbci.logicOpEnable   = false;
		cbci.attachmentCount = 1;
		cbci.pAttachments    = &blendInfo;

		vk::DynamicState ds[] = { 
			vk::DynamicState::eScissor, 
			vk::DynamicState::eViewport
		};

		vk::PipelineDynamicStateCreateInfo dsci{};
		dsci.dynamicStateCount = 2;
		dsci.pDynamicStates    = ds;

		vk::PipelineLayoutCreateInfo plci{};
		m_layout = check_vk(m_dev.createPipelineLayout(plci), "Failed to create pipeline layout");
		create_renderpass();

		//TODO: Pipeline cache
		vk::GraphicsPipelineCreateInfo ci{};
		ci.subpass             = 0;
		ci.layout              = m_layout;
		ci.renderPass          = m_rpass;
		ci.stageCount          = 2;
		ci.pStages             = stages;
		ci.pVertexInputState   = &vci;
		ci.pInputAssemblyState = &iaci;
		ci.pViewportState      = &vpci;
		ci.pColorBlendState    = &cbci;
		ci.pMultisampleState   = &msci;
		ci.pRasterizationState = &rci;
		ci.pDynamicState       = &dsci;
		m_handle = check_vk(m_dev.createGraphicsPipeline(nullptr, ci), "Failed to create graphics pipeline");

		m_dev.destroyShaderModule(vshader);
		m_dev.destroyShaderModule(fshader);
		m_framebufs.resize(m_swapchain.get_image_views().size());
		create_framebuffers();
	}
	
	Pipeline::~Pipeline()
	{
		for(auto fb : m_framebufs) {
			m_dev.destroyFramebuffer(fb);
		}

		m_dev.destroyPipeline(m_handle);
		m_dev.destroyPipelineLayout(m_layout);
		m_dev.destroyRenderPass(m_rpass);
	}

	void Pipeline::bind_cmd(vk::CommandBuffer buf) const
	{
		vk::ClearValue cv{{std::array<float, 4>{0.0f, 0.5f, 0.0f, 1.0f}}};

		vk::RenderPassBeginInfo rbi{};
		rbi.renderPass        = m_rpass;
		rbi.clearValueCount   = 1;
		rbi.pClearValues      = &cv;
		rbi.renderArea.offset = vk::Offset2D{0, 0};
		rbi.renderArea.extent = vk::Extent2D{m_swapchain.get_extent()};
		rbi.framebuffer       = m_framebufs[m_swapchain.get_current_image_index()];
		buf.beginRenderPass(rbi, vk::SubpassContents::eInline);
		buf.bindPipeline(vk::PipelineBindPoint::eGraphics, m_handle);
		
		vk::Viewport vp{};
		vp.x        = 0;
		vp.y        = 0;
		vp.minDepth = 0.0f;
		vp.maxDepth = 1.0f;
		vp.width    = static_cast<float>(m_swapchain.get_extent().width);
		vp.height   = static_cast<float>(m_swapchain.get_extent().height);
		buf.setViewport(0, { vp });

		vk::Rect2D scis{};
		scis.extent = m_swapchain.get_extent();
		buf.setScissor(0, { scis});
	}

	void Pipeline::unbind_cmd(vk::CommandBuffer buf) const
	{
		buf.endRenderPass();
	}
	
	void Pipeline::reset()
	{
		for(auto fb : m_framebufs) {
			m_dev.destroyFramebuffer(fb);
		}

		m_dev.destroyRenderPass(m_rpass);
		create_renderpass();
		create_framebuffers();
	}
	
	void Pipeline::create_renderpass()
	{
		vk::AttachmentReference colref{};
		colref.attachment = 0;
		colref.layout     = vk::ImageLayout::eColorAttachmentOptimal;

		vk::AttachmentDescription attachDesc{};
		attachDesc.format         = m_swapchain.get_format();
		attachDesc.samples        = vk::SampleCountFlagBits::e1; //TODO: Revisit when MS
		attachDesc.loadOp         = vk::AttachmentLoadOp::eClear;
		attachDesc.storeOp        = vk::AttachmentStoreOp::eStore;
		attachDesc.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
		attachDesc.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachDesc.initialLayout  = vk::ImageLayout::eUndefined;
		attachDesc.finalLayout    = vk::ImageLayout::ePresentSrcKHR;

		vk::SubpassDependency sdep{};
		sdep.srcSubpass       = VK_SUBPASS_EXTERNAL;
		sdep.dstSubpass       = 0;
		sdep.srcAccessMask    = vk::AccessFlagBits::eNone;
		sdep.dstAccessMask    = vk::AccessFlagBits::eColorAttachmentWrite;
		sdep.srcStageMask     = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		sdep.dstStageMask     = vk::PipelineStageFlagBits::eColorAttachmentOutput;

		vk::SubpassDescription subpass{};
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments    = &colref;
		subpass.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;

		vk::RenderPassCreateInfo rci{};
		rci.attachmentCount = 1;
		rci.pAttachments    = &attachDesc;
		rci.subpassCount    = 1;
		rci.pSubpasses      = &subpass;
		rci.dependencyCount = 1;
		rci.pDependencies   = &sdep;
		m_rpass = check_vk(m_dev.createRenderPass(rci), "Failed to create renderpass");
	}

	void Pipeline::create_framebuffers()
	{
		auto imviews = m_swapchain.get_image_views();
		for(size_t i = 0; i < imviews.size(); i++) {
			vk::ImageView attachments[] = { imviews[i] };

			vk::FramebufferCreateInfo ci{};
			ci.layers          = 1;
			ci.renderPass      = m_rpass;
			ci.attachmentCount = 1;
			ci.pAttachments    = attachments;
			ci.width           = m_swapchain.get_extent().width;
			ci.height          = m_swapchain.get_extent().height;
			m_framebufs[i] = check_vk(m_dev.createFramebuffer(ci), "Cannot create pipeline framebuffer");
		}
	}
}
