cmake_minimum_required(VERSION 3.5)

project(grfcodec)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_EXTENSIONS NO)

find_package(Git REQUIRED)

set(GENERATED_BINARY_DIR "${CMAKE_BINARY_DIR}/generated")
include_directories("${GENERATED_BINARY_DIR}")

add_custom_command(OUTPUT "${GENERATED_BINARY_DIR}/version.h"
	COMMAND ${CMAKE_COMMAND}
		-D GIT_EXECUTABLE=${GIT_EXECUTABLE}
		-D INPUT_FILE=${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
		-D OUTPUT_FILE=${GENERATED_BINARY_DIR}/version.h
		-P ${CMAKE_CURRENT_SOURCE_DIR}/generate_version.cmake
	DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in"
)
add_custom_target(version_header DEPENDS "${GENERATED_BINARY_DIR}/version.h")

find_package(PNG)

include(TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
	add_definitions(-DGRFCODEC_BIG_ENDIAN=1)
else()
	add_definitions(-DGRFCODEC_LITTLE_ENDIAN=1)
endif()

set(PALETTE_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/tt1_mars.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/tt1_norm.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttd_cand.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttd_norm.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttw_cand.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttw_norm.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttw_pb_pal1.bcp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pals/ttw_pb_pal2.bcp
)

add_custom_command(OUTPUT ${GENERATED_BINARY_DIR}/ttdpal.h
	COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_BINARY_DIR}
	COMMAND perl ${CMAKE_CURRENT_SOURCE_DIR}/src/pal2c.pl ${PALETTE_SOURCE_FILES} > ${GENERATED_BINARY_DIR}/ttdpal.h
	DEPENDS ${PALETTE_SOURCE_FILES}
)
add_custom_target(palettes_header DEPENDS ${GENERATED_BINARY_DIR}/ttdpal.h)

set(GRFID_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/grfid.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/md5.cpp
)
add_executable(grfid ${GRFID_SOURCE_FILES})
add_dependencies(grfid version_header)

set(GRFSTRIP_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/grfstrip.cpp
)
add_executable(grfstrip ${GRFSTRIP_SOURCE_FILES})
add_dependencies(grfstrip version_header)

set(NFORENUM_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/act0.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/act123_classes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/act123.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/act14.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/act5.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/act6.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/act79D.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/actB.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/actF.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/command.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/data.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/globals.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/help.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/IDs.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/inject.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/language_mgr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/mapescapes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/message_mgr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/messages.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/nforenum.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pseudo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pseudo_seq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/rangedint.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/sanity.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/strings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/utf8.cpp
)
add_executable(nforenum ${NFORENUM_SOURCE_FILES})
add_dependencies(nforenum palettes_header version_header)

set(GRFCODEC_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/src/error.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/file.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/globals.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/grfcodec.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/grfcomm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/info.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/mapescapes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/path.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pcxfile.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pcxsprit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/pngsprit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/readinfo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/sprites.cpp
)
add_executable(grfcodec ${GRFCODEC_SOURCE_FILES})
add_dependencies(grfcodec palettes_header version_header)
if(PNG_FOUND)
	set_target_properties(grfcodec PROPERTIES COMPILE_FLAGS -DWITH_PNG)
	target_link_libraries(grfcodec ${PNG_LIBRARY})
endif()


