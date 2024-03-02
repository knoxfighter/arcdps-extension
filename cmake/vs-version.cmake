function(test)
	# set() all needed variables here, they are only set in function scope
	set(options)
	set(oneValueKeywords VERSION_MAJOR VERSION_MINOR VERSION_PATCH VERSION_REF INTERNAL_NAME COPYRIGHT ORIGINAL_FILENAME PRODUCT_NAME)
	set(multiValueKeywords)

	# cmake_parse_arguments
	cmake_parse_arguments(PROJECT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	# check variables that are not optional
	if (NOT DEFINED PROJECT_VERSION_MAJOR)
		message(FATAL_ERROR "VERSION_MAJOR has to be defined!")
	endif ()

	if (NOT DEFINED PROJECT_VERSION_MINOR)
		message(FATAL_ERROR "VERSION_MINOR has to be defined!")
	endif ()

	if (NOT DEFINED PROJECT_VERSION_PATCH)
		message(FATAL_ERROR "VERSION_PATCH has to be defined!")
	endif ()

	if (NOT DEFINED PROJECT_VERSION_REF)
		message(FATAL_ERROR "VERSION_REF has to be defined!")
	endif ()

	if (NOT DEFINED PROJECT_INTERNAL_NAME)
		message(FATAL_ERROR "INTERNAL_NAME has to be defined!")
	endif ()

	if (NOT DEFINED PROJECT_COPYRIGHT)
		message(FATAL_ERROR "COPYRIGHT has to be defined!")
	endif ()

	if (NOT DEFINED PROJECT_ORIGINAL_FILENAME)
		message(FATAL_ERROR "ORIGINAL_FILENAME has to be defined!")
	endif ()

	if (NOT DEFINED PROJECT_PRODUCT_NAME)
		message(FATAL_ERROR "PRODUCT_NAME has to be defined!")
	endif ()

	configure_file("${CMAKE_CURRENT_FUNCTION_LIST_DIR}/VersionInfo.rc.in" VersionInfo.rc @ONLY)

	message(FATAL_ERROR "test was called!")
endfunction()
