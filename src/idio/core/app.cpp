/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"

#include <cassert>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "gfx/context.hpp"
#include "gfx/swapchain.hpp"

namespace idio
{
	Logger s_EngineLogger = nullptr;
	Application *Application::s_Instance = nullptr;

	Application::Application(std::string name, Version v, const WindowCreateInfo &wci) :
		m_version(v), m_name(std::move(name)), m_windowCreateInfo(wci)
	{
		assert(s_Instance == nullptr);
		s_Instance = this;
	}

	void Application::run()
	{
		m_mainWindow = std::make_unique<Window>(m_windowCreateInfo);
		m_context = std::make_unique<Context>(m_version, m_name, *m_mainWindow);
		m_mainWindow->create_swapchain(*m_context);
		init();

		bool minimised = false;
		while(m_open) {
			if(!minimised) {
				if(!m_mainWindow->clear()) {
					recreate_pipelines();
					continue;
				}

				tick();
			}

			// I know...
			SDL_Event sdlEvt;
			while(SDL_PollEvent(&sdlEvt)) {
				auto evt = translate_evt(sdlEvt);
				bool handled = evt_handler(
					evt,
					[&](const QuitEvent &qe) -> bool {
						m_open = false;
						return true;
					},

					[&](const WindowMinimiseEvent &me) -> bool {
						if(me.id == m_mainWindow->get_id()) {
							minimised = me.minimised;
							return true;
						}

						return false;
					},
					[&](const WindowClosedEvent &wce) -> bool {
						if(wce.id == m_mainWindow->get_id()) {
							m_open = false;
							return true;
						}

						return false;
					},
					[&](const WindowResizeEvent &wre) -> bool {
						if(wre.id == m_mainWindow->get_id()) {
							m_mainWindow->create_swapchain(*m_context);
							recreate_pipelines();
							return true;
						}

						return false;
					});

				if(!handled) {
					event_proc(evt);
				}
			}
		}
	}

	void Application::close()
	{
		s_Instance->m_open = false;
	}

	Application &Application::get()
	{
		return *s_Instance;
	}

	void Application::crash() noexcept
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Critical error :) (Check logs)", nullptr);
		s_EngineLogger->critical("Whoopsie we crashed! Check the logs :)");
		s_EngineLogger->flush();
		std::terminate();
	}


	Logger make_logger(const std::string &pref_dir, const std::string &name)
	{
		static auto logdir = fmt::format("{}/latest.log", pref_dir);
		static std::array<spdlog::sink_ptr, 2> sinks {
			std::make_shared<spdlog::sinks::stdout_color_sink_mt>(),
			std::make_shared<spdlog::sinks::basic_file_sink_mt>(logdir, true)
		};

		sinks[0]->set_pattern("%^[%T] %n: %v%$");
		sinks[1]->set_pattern("[%T] [%l] %n: %v");

		auto hdl = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
		spdlog::register_logger(hdl);
		hdl->flush_on(spdlog::level::trace);
		if constexpr(ID_DEBUG) {
			hdl->set_level(spdlog::level::trace);
		} else {
			hdl->set_level(spdlog::level::info);
		}

		return hdl;
	}


	namespace internal
	{
		void init_engine()
		{
			auto &app = Application::get();
			auto appname = app.get_name();

			char *prefpath = SDL_GetPrefPath("idio", appname.c_str());
			if(prefpath == nullptr) {
				std::cout << "[Early init error]: " << SDL_GetError() << std::endl;
				std::exit(EXIT_FAILURE);
			}

			app.m_prefpath = prefpath;
			SDL_free(prefpath);

			s_EngineLogger = make_logger(app.get_pref_dir(), "Idio");
			if(SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) != 0) {
				s_EngineLogger->critical("Failed to init SDL: {}", SDL_GetError());
				Application::crash();
			}
		}

		void deinit_engine()
		{
			SDL_Quit();
		}
	}
}

void idio_main(int argc, char **argv)
{
	std::span<char *> args(argv, static_cast<size_t>(argc));
	idio::Application *app = idio::make_application(args);
	idio::internal::init_engine();
	app->run();
	delete app;
	idio::internal::deinit_engine();
}