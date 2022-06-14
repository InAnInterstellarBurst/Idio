# Copyright (c) 2022 Connor Mellon
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(WIN32)
	set(OS_SRC core/win32.cpp)
else()
	set(OS_SRC core/linux.cpp)
endif()

file(GLOB CORE_GLOB core/*.cpp core/*.hpp)

set(SRCS
	pch.hpp idio.hpp
	${CORE_GLOB}
	${OS_SRC}
	.licenseheader
)

add_library(idio STATIC ${SRCS})
target_precompile_headers(idio PRIVATE pch.hpp)

target_link_libraries(idio
	PRIVATE
		project_settings
	PUBLIC
		Vulkan::Vulkan
		spdlog
)

target_include_directories(idio INTERFACE ${CMAKE_SOURCE_DIR})
target_include_directories(idio PUBLIC ${CMAKE_SOURCE_DIR}/idio)
target_include_directories(idio PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRCS})
set_property(TARGET idio PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
