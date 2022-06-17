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

	class Context
	{
	public:
		Context(const ApplicationInfo& ai);
		~Context();
	private:
		vk::Instance m_instance;
		std::unique_ptr<vk::DispatchLoaderDynamic> m_dispatchLoader;

#if ID_DEBUG
		vk::DebugUtilsMessengerEXT m_dbgmsgr;
#endif
	};
}