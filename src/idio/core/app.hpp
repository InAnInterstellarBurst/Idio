/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IDIO_CORE_APP_H
#define IDIO_CORE_APP_H

#include "types.hpp"
#include "event.hpp"
#include "window.hpp"

extern void idio_main(int argc, char **argv);

namespace idio
{
	class Context;

	class Application
	{
	public:
		Application(std::string name, Version v, const WindowCreateInfo &wci);
		virtual ~Application() = default;
		Application(const Application &o) = delete;
		Application &operator=(const Application &o) = delete;

		void run();
		virtual void event_proc(const Event &e) = 0;

		std::string get_name() const { return m_name; }
		std::string get_pref_dir() const { return m_prefpath; }

		static void close();
		static Application &get();
		[[noreturn]] static void crash() noexcept;
	protected:
		bool m_open = true;
		Version m_version;
		std::string m_name;
		std::string m_prefpath;

		const WindowCreateInfo m_windowCreateInfo;
		Logger m_gameLogger;
		std::unique_ptr<Context> m_context;
		std::unique_ptr<Window> m_mainWindow;

		virtual void init() = 0;
		virtual void tick() = 0;
		virtual void recreate_pipelines() = 0;
	private:
		static Application *s_Instance;

		friend void internal::init_engine();
		friend void internal::deinit_engine();
		friend void ::idio_main(int argc, char **argv);
	};

	Logger make_logger(const std::string &pref_dir, const std::string &name);

	extern Application *make_application(std::span<char *> cmdargs);
}

#endif
