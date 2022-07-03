# Copyright (c) 2022 Connor Mellon
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(SRCS_CORE
	app.hpp app.cpp event.hpp
	window.cpp window.hpp types.hpp
)

set(SRCS_GFX
	vkutl.hpp
	context.hpp context.cpp
	swapchain.hpp swapchain.cpp
	pipeline.hpp pipeline.cpp
	buffer.hpp buffer.cpp
)

if(WIN32)
	list(APPEND SRCS_CORE "win32.cpp")
else()
	list(APPEND SRCS_CORE "linux.cpp")
endif()

list(TRANSFORM SRCS_CORE PREPEND "core/")
list(TRANSFORM SRCS_GFX PREPEND "gfx/")
add_library(idio STATIC
	.licenseheader
	pch.hpp idio.hpp
	"${SRCS_CORE}"
	"${SRCS_GFX}"
)

target_precompile_headers(idio PRIVATE pch.hpp)
target_link_libraries(idio
	PRIVATE
	project_settings
	PUBLIC
	glm
	VulkanMemoryAllocator
	spdlog
	SDL2-static
	Vulkan::Vulkan
)

target_include_directories(idio INTERFACE ${CMAKE_SOURCE_DIR}/src/)
target_include_directories(idio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
set_property(TARGET idio PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
