/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <span>
#include <array>
#include <tuple>
#include <string>
#include <memory>
#include <vector>
#include <memory>
#include <utility>
#include <iostream>
#include <string_view>

#include <SDL.h>
#include <fmt/format.h>

#include "core/logger.hpp"
#include "core/event.hpp"
#include "core/window.hpp"
#include "core/app.hpp"

namespace Idio
{
	extern void main(const std::span<char*>& args);
	
	constexpr std::tuple<uint32_t, uint32_t, uint32_t> g_EngineVersion { 0, 0, 0 };
}