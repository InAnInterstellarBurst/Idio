/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef IDIO_GFX_VKUTL_H
#define IDIO_GFX_VKUTL_H

namespace idio
{
	template<typename T>
	[[nodiscard]] T check_vk(const vk::ResultValue<T> &rv, std::string_view msg)
	{
		if(rv.result != vk::Result::eSuccess) {
			s_EngineLogger->critical("[Vulkan]: {}, {}", vk::to_string(rv.result), msg);
			Application::crash();
		}

		return rv.value;
	}

	inline void check_vk(vk::Result r, std::string_view msg)
	{
		if(r != vk::Result::eSuccess) {
			s_EngineLogger->critical("[Vulkan]: {}, {}", vk::to_string(r), msg);
			Application::crash();
		}
	}

	inline void check_vk(VkResult r, std::string_view msg)
	{
		check_vk(vk::Result { r }, msg);
	}
}

#endif
