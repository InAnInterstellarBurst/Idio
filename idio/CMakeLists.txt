# Copyright (c) 2022 Connor Mellon
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

file(GLOB CORE_GLOB core/*.cpp core/*.hpp)
if(WIN32)
	list(FILTER CORE_GLOB EXCLUDE REGEX ".*linux\\.cpp$")
else()
	list(FILTER CORE_GLOB EXCLUDE REGEX ".*win32\\.cpp$")
endif()

set(SRCS
	pch.hpp idio.hpp
	${CORE_GLOB}
	.licenseheader
)

add_library(idio STATIC ${SRCS})
target_precompile_headers(idio PRIVATE pch.hpp)

target_link_libraries(idio
	PRIVATE
		fmt::fmt
		project_settings
	PUBLIC
		SDL2-static
		Vulkan::Vulkan
)

target_include_directories(idio INTERFACE ${CMAKE_SOURCE_DIR})
target_include_directories(idio PUBLIC ${CMAKE_SOURCE_DIR}/idio)
target_include_directories(idio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
set_property(TARGET idio PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
