# Copyright (c) 2022 Connor Mellon
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.18)
project(idio)

set(LIBC ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Release")
endif()

include(cmake/tools.cmake)
include(cmake/pvt_is.cmake)
include(cmake/warnings.cmake)

add_library(project_settings INTERFACE)
enable_sanitisers(project_settings)
set_project_warnings(project_settings)
target_compile_features(project_settings INTERFACE cxx_std_20)
target_compile_definitions(project_settings INTERFACE ID_DEBUG=$<CONFIG:DEBUG> _CRT_SECURE_NO_WARNINGS)

if(WIN32)
	target_compile_definitions(project_settings INTERFACE ID_DESKTOP=1 ID_WIN32=1 ID_LINUX=0)
elseif(LINUX)
	target_compile_definitions(project_settings INTERFACE ID_DESKTOP=1 ID_LINUX=1 ID_WIN32=0)
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	target_compile_definitions(project_settings INTERFACE ID_DEBUG=1)
else()
	target_compile_definitions(project_settings INTERFACE ID_DEBUG=0)
endif()


add_subdirectory(dep/spdlog)
find_package(Vulkan REQUIRED)

add_subdirectory(idio)
if(NOT ID_NOTEST)
	add_subdirectory(testapp)
endif()

set_target_properties(spdlog PROPERTIES FOLDER "Dependencies")
