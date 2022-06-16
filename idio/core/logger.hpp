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

	class Logger
	{
		Logger(std::string name, const ApplicationInfo& appinfo);
		~Logger();

	private:
		std::string m_name;

		void log_base(const std::string& msg);
		void 

		static std::ofstream s_Logfile;
	};
}