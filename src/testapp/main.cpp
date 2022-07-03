/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <idio/idio.hpp>
#include <glm/glm.hpp>

using namespace idio;

struct Vertex
{
	glm::vec2 pos;
	glm::vec3 col;

	Vertex(float x, float y, float r, float g, float b) :
		pos(x, y), col(r, g, b) {}
};

class App : public Application
{
public:
	App(std::string name, Version v, const WindowCreateInfo &wci) :
		Application(name, v, wci)
	{
	}

	~App()
	{
		check_vk(m_context->get_device().waitIdle(), "Got impatient?");
	}
protected:
	void init()
	{
		auto vscode = load_shader_from_disk("./shaders/basic.vert.spv");
		auto fscode = load_shader_from_disk("./shaders/basic.frag.spv");
		if(!vscode || !fscode) {
			m_gameLogger->critical("Oh dear one of the shaders are invalid");
			crash();
		}

		PipelineCreateInfo pci {};
		pci.vertexShaderCode = *vscode;
		pci.fragmentShaderCode = *fscode;

		pci.vertexLayouts = {
			VertexLayout {
				.stride = sizeof(Vertex),
				.binding = 0,
			}
		};

		pci.attributeDescs = {
			AttributeDescription {
				.offset = 0,
				.binding = 0,
				.location = 0,
				.format = AttribFormat::Vec2,
			},
			AttributeDescription {
				.offset = offsetof(Vertex, col),
				.binding = 0,
				.location = 1,
				.format = AttribFormat::Vec3,
			}
		};

		m_pipeline = std::make_unique<Pipeline>(m_context->get_device(),
			m_mainWindow->get_swapchain(), pci);

		m_cmdpool = std::make_unique<CommandPool>(*m_context);
		m_cmdbufs = m_cmdpool->get_buffers(s_MaxFramesProcessing);

		const std::vector<Vertex> verts {
			{ 0.0f, -0.5f, 1.0f, 0.0f, 0.0f },
			{ 0.5f, 0.5f, 1.0f, 1.0f, 0.0f },
			{ -0.5f, 0.0f, 1.0f, 1.0f, 1.0f }
		};
	}

	void tick()
	{
		auto cmdbuf = m_cmdbufs[m_mainWindow->get_swapchain().get_current_frame_index()];
		cmdbuf.reset();
		m_context->begin_cmd(cmdbuf);
		m_pipeline->bind_cmd(cmdbuf);
		m_context->draw_cmd(cmdbuf, 3);
		m_pipeline->unbind_cmd(cmdbuf);
		m_context->end_cmd(cmdbuf);
		m_context->submit_gfx_queue(m_mainWindow->get_swapchain(), { cmdbuf });

		static std::vector<Swapchain *> scs { &m_mainWindow->get_swapchain() };
		Swapchain::present(*m_context, scs);
	}

	void recreate_pipelines()
	{
		m_pipeline->reset();
	}

	void event_proc(const Event &e)
	{
		evt_handler(
			e,
			[](const QuitEvent &qe) -> bool { return true; },
			[](const WindowClosedEvent &ce) -> bool { return true; },
			[](const WindowResizeEvent &re) -> bool { return true; },
			[](const WindowMinimiseEvent &me) -> bool { return true; });
	}
private:
	std::unique_ptr<Pipeline> m_pipeline;
	std::unique_ptr<CommandPool> m_cmdpool;
	std::vector<vk::CommandBuffer> m_cmdbufs;
};

Application *idio::make_application(std::span<char *> cmdargs)
{
	return new App("Hello", Version { 0, 0, 1 }, WindowCreateInfo { .resizeable = true });
}
