/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IDIO_CORE_TYPES_H
#define IDIO_CORE_TYPES_H

namespace idio
{
	namespace internal
	{
		extern void init_engine();
		extern void deinit_engine();
	}

	struct Version
	{
		uint32_t major;
		uint32_t minor;
		uint32_t patch;
	};

	constexpr Version k_EngineVersion { 0, 0, 1 };

	using Logger = std::shared_ptr<spdlog::logger>;
	extern Logger s_EngineLogger;
}

#endif
