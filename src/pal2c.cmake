cmake_minimum_required(VERSION 3.5)

set(ARGC 1)
set(ARG_READ NO)

# Read all the arguments given to CMake; we are looking for -- and everything
# that follows. Those are our palette files.
while(ARGC LESS CMAKE_ARGC)
	set(ARG ${CMAKE_ARGV${ARGC}})

	if(ARG_READ)
		list(APPEND PALETTE_FILES "${ARG}")
	endif()

	if(ARG STREQUAL "--")
		set(ARG_READ YES)
	endif()

	math(EXPR ARGC "${ARGC} + 1")
endwhile()

set(PALNUM 0)
foreach(PALFILE IN LISTS PALETTE_FILES)
	get_filename_component(PAL ${PALFILE} NAME_WE)
	string(APPEND DEFINES "#define PAL_${PAL} ${PALNUM}\n")
	math(EXPR PALNUM "${PALNUM} + 1")
	string(APPEND PALETTES "// PAL_${PAL}\n    {\n")

	set(COUNT 0)
	string(LENGTH "${PALETTE}" LEN)
	math(EXPR LEN "${LEN} / 6")
	while(COUNT LESS 256)
		math(EXPR OFFSET "${COUNT} * 3")
		file(READ ${PALFILE} PALETTE OFFSET ${OFFSET} LIMIT 3 HEX)
		string(REGEX REPLACE "(..)" "0x\\1," PALETTE ${PALETTE})
		math(EXPR COUNT "${COUNT} + 1")
		math(EXPR MOD "${COUNT} % 4")
		if(MOD EQUAL 1)
			string(APPEND PALETTES "\t")
		endif()
		string(APPEND PALETTES "${PALETTE}   ")
		if(MOD EQUAL 0)
			math(EXPR FROM "${COUNT} - 4")
			math(EXPR TO "${COUNT} - 1")
			string(APPEND PALETTES "\t// ${FROM}-${TO}\n")
		endif()
	endwhile()
	string(APPEND PALETTES "    },\n")
endforeach()

configure_file(${INPUT_FILE} ${OUTPUT_FILE})
