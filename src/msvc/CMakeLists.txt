cmake_minimum_required(VERSION 3.5)

if(MSVC)
	# Add our own getopt to grfcodec
	target_sources(grfcodec PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/getopt.cpp)
	target_include_directories(grfcodec PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

	# Add our own getopt to nforenum
	target_sources(nforenum PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/getopt.cpp)
	target_include_directories(nforenum PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
