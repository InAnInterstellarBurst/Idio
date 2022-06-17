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
	template<typename T>
	[[nodiscard]] T check_vk(const vk::ResultValue<T>& rv)
	{
		if(rv.result != vk::Result::eSuccess) {
			s_EngineLogger->critical("[Vulkan]: {}", vk::to_string(rv.result));
			crash();
		}

		return rv.value;
	}
}