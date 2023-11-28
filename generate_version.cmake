find_package(Git QUIET)

# .git may be a directory or a regular file
if(GIT_FOUND AND EXISTS ".git")
	execute_process(
		COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty=M
		OUTPUT_VARIABLE GIT_VERSION
		RESULT_VARIABLE GIT_DESCRIBE_RESULT
		ERROR_VARIABLE GIT_DESCRIBE_ERROR
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)

	execute_process(
		COMMAND ${GIT_EXECUTABLE} show -s --pretty=%cd --date=short HEAD
		OUTPUT_VARIABLE GIT_DATE
		RESULT_VARIABLE GIT_SHOW_RESULT
		ERROR_VARIABLE GIT_SHOW_ERROR
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
elseif(EXISTS ".version")
	file(READ ".version" GIT_VERSION)
	string(REPLACE "\n" "" GIT_VERSION "${GIT_VERSION}")
	string(REPLACE "\t" ";" GIT_VERSION "${GIT_VERSION}")
	list(GET GIT_VERSION 1 GIT_DATE)
	list(GET GIT_VERSION 0 GIT_VERSION)
else()
	message(WARNING "No version detected")
	set(GIT_VERSION "unknown")
	set(GIT_DATE "unknown")
endif()

if(WRITE_VERSION)
	file(WRITE .version "${GIT_VERSION}\t${GIT_DATE}\n")
else()
	message(STATUS "Generating version.h")
	configure_file("src/version.h.in" "${GENERATED_BINARY_DIR}/version.h")

	message(STATUS "Generating CPackProperties.cmake")
	configure_file("CPackProperties.cmake.in" "${CPACK_BINARY_DIR}/CPackProperties.cmake" @ONLY)
endif()
