cmake_minimum_required(VERSION 3.5)

project(grfcodec)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_EXTENSIONS NO)

if(MINGW)
	# Force searching static libs, so the executables can run outside MinGW environment
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

	# Force static linking, so the executables can run outside MinGW environment
	link_libraries(-static -static-libgcc -static-libstdc++)
endif()

if(MSVC)
	# Switch to MT (static) instead of MD (dynamic) binary

	# For MSVC two generators are available
	# - a command line generator (Ninja) using CMAKE_BUILD_TYPE to specify the
	#   configuration of the build tree
	# - an IDE generator (Visual Studio) using CMAKE_CONFIGURATION_TYPES to
	#   specify all configurations that will be available in the generated solution
	list(APPEND MSVC_CONFIGS "${CMAKE_BUILD_TYPE}" "${CMAKE_CONFIGURATION_TYPES}")

	# Set usage of static runtime for all configurations
	foreach(MSVC_CONFIG ${MSVC_CONFIGS})
		string(TOUPPER "CMAKE_CXX_FLAGS_${MSVC_CONFIG}" MSVC_FLAGS)
		string(REPLACE "/MD" "/MT" ${MSVC_FLAGS} "${${MSVC_FLAGS}}")
	endforeach()

	add_compile_options(
		/wd4996 # Disable deprecation warnings
		/W3
	)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
	add_compile_options(
		-Wall
		-Wextra
		-Wno-format-nonliteral
	)
endif()


# Add some -D flags for Debug builds. We cannot use add_definitions(), because
# it does not appear to support the $<> tags.
add_compile_options(
    "$<$<CONFIG:Debug>:-D_DEBUG>"
    "$<$<NOT:$<CONFIG:Debug>>:-D_FORTIFY_SOURCE=2>" # FORTIFY_SOURCE should only be used in non-debug builds (requires -O1+)
)
if(MINGW)
    add_link_options(
        "$<$<NOT:$<CONFIG:Debug>>:-fstack-protector>" # Prevent undefined references when _FORTIFY_SOURCE > 0
    )
endif()


find_package(Git REQUIRED)
find_package(PNG)
find_package(Boost REQUIRED)


include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
	add_definitions(-DGRFCODEC_BIG_ENDIAN=1)
else()
	add_definitions(-DGRFCODEC_LITTLE_ENDIAN=1)
endif()


include(CheckCXXSymbolExists)
check_cxx_symbol_exists("fmemopen" "cstdio" HAS_FMEMOPEN)
if(HAS_FMEMOPEN)
	add_definitions(-DWITH_FMEMOPEN)
endif()


# Prepare generated dir
set(GENERATED_BINARY_DIR "${CMAKE_BINARY_DIR}/generated")
include_directories("${GENERATED_BINARY_DIR}")

# Target to generate version.h
add_custom_target(version_header
	COMMAND ${CMAKE_COMMAND}
		-D GIT_EXECUTABLE=${GIT_EXECUTABLE}
		-D INPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
		-D OUTPUT_FILE=${GENERATED_BINARY_DIR}/version.h
		-P "${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake"
	BYPRODUCTS "${GENERATED_BINARY_DIR}/version.h"
)

# Target to generate ttpal.h
set(PALETTE_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttd_norm.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttw_norm.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttd_cand.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttw_cand.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/tt1_norm.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/tt1_mars.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttw_pb_pal1.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttw_pb_pal2.bcp
)
add_custom_target(palettes_header
	COMMAND ${CMAKE_COMMAND}
		-D INPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/src/ttdpal.h.in
		-D OUTPUT_FILE=${GENERATED_BINARY_DIR}/ttdpal.h
		-P "${CMAKE_CURRENT_SOURCE_DIR}/src/pal2c.cmake"
		--
		${PALETTE_SOURCE_FILES}
	DEPENDS ${PALETTE_SOURCE_FILES}
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/ttdpal.h.in"
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/pal2c.cmake"
	BYPRODUCTS "${GENERATED_BINARY_DIR}/ttdpal.h"
)


# Create grfid
add_executable(grfid)
add_dependencies(grfid version_header)


# Create grfstrip
add_executable(grfstrip)
add_dependencies(grfstrip version_header)


# Create nforenum
add_executable(nforenum)
add_dependencies(nforenum palettes_header version_header)
target_link_libraries(nforenum Boost::boost)


# Create grfcodec
add_executable(grfcodec)
add_dependencies(grfcodec palettes_header version_header)
target_link_libraries(grfcodec Boost::boost)
if(PNG_FOUND)
	set_target_properties(grfcodec PROPERTIES COMPILE_FLAGS -DWITH_PNG)
	target_link_libraries(grfcodec PNG::PNG)
endif()


# Add source files
add_subdirectory(src)


# Install files
include(GNUInstallDirs)

install(TARGETS
	grfid
	grfstrip
	nforenum
	grfcodec
	DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES
	${CMAKE_CURRENT_SOURCE_DIR}/changelog.txt
	${CMAKE_CURRENT_SOURCE_DIR}/COPYING
	DESTINATION ${CMAKE_INSTALL_DOCDIR}
)

add_subdirectory(docs)
