# Copyright (c) 2022 Connor Mellon
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Enable ccache
find_program(CCACHE ccache)

if(CCACHE)
	set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE})
else()
	message(WARNING "CCache not found")
endif()

# Sanitisers
function(enable_sanitisers target_name)
	if(CMAKE_C_COMPILER_ID STREQUAL "MSVC" OR NOT ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
		return()
	endif()

	set(SANITISERS "")

	if(NOT ENABLE_THREAD_SANITISERS)
		set(SANITISERS "address,undefined,leak")
	else()
		set(SANITISERS "thread")
	endif()

	target_link_options(${target_name} INTERFACE -fsanitize=${SANITISERS})
	target_compile_options(${target_name} INTERFACE -fsanitize=${SANITISERS})
endfunction()

# Colour diagnostics
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_C_COMPILER_ID MATCHES ".*Clang")
	add_compile_options(-fcolor-diagnostics)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-fdiagnostics-color=always)
endif()

# Visual studio settings
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

function(make_startup_target target)
	set_property(TARGET ${target} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${target}>)
	set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target})
endfunction()

# Linker
macro(set_alternate_linker linker)
	find_program(LINKER_EXECUTABLE ld.${ID_ALTERNATE_LINKER} ${ID_ALTERNATE_LINKER})

	if(LINKER_EXECUTABLE)
		if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND "${CMAKE_CXX_COMPILER_VERSION}" VERSION_LESS 12.0.0)
			add_link_options("-ld-path=${ID_ALTERNATE_LINKER}")
		else()
			add_link_options("-fuse-ld=${ID_ALTERNATE_LINKER}")
		endif()
	else()
		set(ID_ALTERNATE_LINKER "" CACHE STRING "Use alternate linker" FORCE)
	endif()
endmacro()

if(NOT "${ID_ALTERNATE_LINKER}" STREQUAL "")
	set_alternate_linker(${ID_ALTERNATE_LINKER})
endif()

# IPO
include(CheckIPOSupported)

function(enable_ipo target)
	if(ID_USE_IPO)
		check_ipo_supported(RESULT result OUTPUT output)

		if(result)
			message("${target} is using IPO!")
			set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
		else()
			message(WARNING "IPO is not supported: ${output}")
		endif()
	endif()
endfunction()