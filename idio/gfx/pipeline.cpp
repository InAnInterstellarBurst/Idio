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
	std::vector<uint32_t> load_shader_from_disk(const std::string& pth)
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
		// Big ass ctor :)
		// Copy paste from old project *ahemahem*
		m_framebufs.reserve(m_swapchain.get_image_views().size());
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
			m_framebufs.push_back(check_vk(m_dev.createFramebuffer(ci),
				"Cannot create pipeline framebuffer"));
		}
	}
}