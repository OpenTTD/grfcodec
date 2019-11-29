cmake_minimum_required(VERSION 3.5)

add_subdirectory(msvc)


# Add files for grfid
target_sources(grfid PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/grfid.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/md5.cpp
)


# Add files for grfstrip
target_sources(grfstrip PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/grfstrip.cpp
)


# Add files for grfcodec
target_sources(grfcodec PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/error.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/file.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/globals.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/grfcodec.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/grfcomm.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/info.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mapescapes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/path.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/pcxfile.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/pcxsprit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/pngsprit.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/readinfo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sprites.cpp
)


# Add files for nforenum
target_sources(nforenum PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/act0.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/act123_classes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/act123.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/act14.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/act5.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/act6.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/act79D.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/actB.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/actF.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/command.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/data.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/globals.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/help.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/IDs.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/inject.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/language_mgr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/mapescapes.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/message_mgr.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/messages.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/nforenum.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/pseudo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/pseudo_seq.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/rangedint.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/sanity.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/strings.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/utf8.cpp
)
