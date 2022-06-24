/**
 * Copyright (c) 2022 Connor Mellon
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <span>
#include <array>
#include <tuple>
#include <string>
#include <memory>
#include <limits>
#include <vector>
#include <memory>
#include <utility>
#include <optional>
#include <iostream>
#include <string_view>

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#include <SDL.h>
#include <fmt/format.h>

#include "core/logger.hpp"