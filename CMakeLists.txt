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
target_compile_definitions(project_settings INTERFACE ID_DEBUG=$<CONFIG:DEBUG> _CRT_SECURE_NO_WARNINGS SDL_MAIN_HANDLED)

if(WIN32)
	target_compile_definitions(project_settings INTERFACE ID_DESKTOP=1 ID_WIN32=1 ID_LINUX=0)
else()
	target_compile_definitions(project_settings INTERFACE ID_DESKTOP=1 ID_LINUX=1 ID_WIN32=0)
endif()

set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
add_subdirectory(dep/fmt)
add_subdirectory(dep/SDL2)
add_subdirectory(dep/vma)
add_subdirectory(dep/glm)
find_package(Vulkan REQUIRED)

add_subdirectory(idio)
enable_ipo(idio)
if(NOT ID_NOTEST)
	add_subdirectory(testapp)
	enable_ipo(testapp)
endif()

set_target_properties(fmt PROPERTIES FOLDER "Dependencies")
set_target_properties(uninstall PROPERTIES FOLDER "Dependencies")
set_target_properties(SDL2main PROPERTIES FOLDER "Dependencies")
set_target_properties(SDL2-static PROPERTIES FOLDER "Dependencies")
set_target_properties(VulkanMemoryAllocator PROPERTIES FOLDER "Dependencies")