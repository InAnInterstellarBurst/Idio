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
	struct ApplicationInfo;

	enum class LogLevel
	{
		Trace,
		Info,
		Warning,
		Error,
		Critical
	};

	[[noreturn]] void crash();

	class Logger
	{
		friend void Idio::crash();
	public:
		Logger(std::string name, const ApplicationInfo& appinfo);

		void set_file_filter(LogLevel l) { m_fileFilter = l; }
		void set_console_filter(LogLevel l) { m_consFilter = l; }

		template<typename... Args>
		void trace(fmt::format_string<Args...> fmt, Args&&... args)
		{
			log(LogLevel::Trace, fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		void info(fmt::format_string<Args...> fmt, Args&&... args)
		{
			log(LogLevel::Info, fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		void warning(fmt::format_string<Args...> fmt, Args&&... args)
		{
			log(LogLevel::Warning, fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		void error(fmt::format_string<Args...> fmt, Args&&... args)
		{
			log(LogLevel::Error, fmt, std::forward<Args>(args)...);
		}
		
		template<typename... Args>
		void critical(fmt::format_string<Args...> fmt, Args&&... args)
		{
			log(LogLevel::Critical, fmt, std::forward<Args>(args)...);
		}
	private:
		std::string m_name;
		LogLevel m_fileFilter;
		LogLevel m_consFilter;

		template<typename... Args>
		void log(LogLevel ll, fmt::format_string<Args...>& fmt, Args&&... args)
		{
			auto fmsg = fmt::format(fmt, std::forward<Args>(args)...);
			auto fopt = fmt::format(s_FileOptFmt, m_name, ll_to_str(ll), fmsg);
			auto copt = fmt::format(s_ConsOptFmt, ll_to_colour(ll), m_name, 
				fmsg, ll_to_colour(LogLevel::Trace));
			log_base(ll, fopt, copt);
		}

		void log_base(LogLevel l, const std::string& fmsg, const std::string& cmsg);

		static std::ofstream s_Logfile;
		constexpr static const char* s_ConsOptFmt = "{}[{}]: {}{}\n";
		constexpr static const char* s_FileOptFmt = "[{}, {}]: {}\n";

		constexpr static const char* ll_to_str(LogLevel l);
		constexpr static const char* ll_to_colour(LogLevel l);
	};

	extern std::unique_ptr<Logger> s_EngineLogger;
}