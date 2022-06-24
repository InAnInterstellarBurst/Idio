# Copyright (c) 2022 Connor Mellon
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

include(${CMAKE_SOURCE_DIR}/cmake/resources.cmake)

if(WIN32)
	set(EXEFLAG WIN32)
endif()

set(SRCS
	main.cpp
	.licenseheader
)

set(SHADERS
	shaders/basic.vert shaders/basic.frag
)

add_executable(testapp ${EXEFLAG} ${SRCS})
make_startup_target(testapp)
target_link_libraries(testapp PRIVATE project_settings idio)
set_property(TARGET ${target} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
add_shaders(testapp "${SHADERS}")
