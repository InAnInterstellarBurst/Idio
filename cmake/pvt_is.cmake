# Copyright (c) 2022 Connor Mellon
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

function(prevent_insource)
	get_filename_component(srcdir "${CMAKE_SOURCE_DIR}" REALPATH)
	get_filename_component(bindir "${CMAKE_BINARY_DIR}" REALPATH)

	if("${srcdir}" STREQUAL "${bindir}")
		message("######################################################")
		message("Note: in-source builds are disabled")
		message("Please create a separate build directory and run cmake from there")
		message("######################################################")
		message(FATAL_ERROR "Quitting configuration")
	endif()
endfunction()

prevent_insource()
