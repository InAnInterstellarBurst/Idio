/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "pch.hpp"
#include <fstream>
#include <exception>
#include "core/app.hpp"

namespace Idio
{
	std::unique_ptr<Logger> s_EngineLogger = nullptr;
	
	void crash()
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Critical error", "Check logs :(", nullptr);
		s_EngineLogger->critical("\n---------------------------------------\n| Engine requested a crash (see logs) |\n---------------------------------------");
		Logger::s_Logfile.close(); // Ensure logs get written
		std::exit(-1);
	}


	std::ofstream Logger::s_Logfile;

	Logger::Logger(std::string name, const ApplicationInfo& appinfo) :
		m_name(std::move(name)), m_fileFilter(LogLevel::Trace),
		m_consFilter(LogLevel::Info)
	{
		if(!s_Logfile.is_open()) {
			s_Logfile.open(appinfo.prefPath + "latest.log", std::ios::trunc | std::ios::out);
		}
	}

	void Logger::log_base(LogLevel l, const std::string& fmsg, 
		const std::string& cmsg)
	{
		if(l >= m_consFilter) {
			std::cout << cmsg;
		}
	
		if(l >= m_fileFilter && s_Logfile.is_open()) {
			s_Logfile << fmsg;
		}
	}

	const char* Logger::ll_to_str(LogLevel l)
	{
		switch(l) {
		case LogLevel::Info:
			return "Info";
		case LogLevel::Warning:
			return "Warning";
		case LogLevel::Error:
			return "Error";
		case LogLevel::Critical:
			return "Critical";
		default:
			return "Trace";
		}
	}

	const char* Logger::ll_to_colour(LogLevel l)
	{
		switch(l) {
		case LogLevel::Info:
			return "\x1b[32m";
		case LogLevel::Warning:
			return "\x1b[33m";
		case LogLevel::Error:
			return "\x1b[31m";
		case LogLevel::Critical:
			return "\x1b[33;41m";
		default:
			return "\x1b[0m";
		}
	}
}