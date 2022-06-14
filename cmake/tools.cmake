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

# (Maybe) Enable doxygen
option(ID_BUILD_DOCS Off)
if(ID_BUILD_DOCS)
	set(DOXYGEN_CALL_GRAPH On)
	set(DOXYGEN_EXTRACT_ALL On)
	set(DOXYGEN_CALLER_GRAPH On)
	find_package(Doxygen REQUIRED dot)
	doxygen_add_docs(doxygen-docs ${CMAKE_SOURCE_DIR}/idio)
endif()

# SANITISERS
option(ENABLE_THREAD_SANITISER Off)

function(enable_sanitisers target_name)
	if(CMAKE_C_COMPILER_ID STREQUAL "MSVC" OR ${CMAKE_BUILD_TYPE} STREQUAL "Release")
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

# IDE STUFF AND COLOUR DIAGS
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(CMAKE_C_COMPILER_ID MATCHES ".*Clang")
	add_compile_options(-fcolor-diagnostics)
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
	add_compile_options(-fdiagnostics-color=always)
endif()

# VISUAL STUDIO SETTINGS
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

function(make_startup_target target)
	set_property(TARGET ${target} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${target}>)
	set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${target})
endfunction()
