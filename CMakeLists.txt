cmake_minimum_required(VERSION 3.7)
project(smart_mirror)

if(WIN32)
	set(CMAKE_FIND_LIBRARY_PREFIXES "" "lib")
	set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".a" ".lib")
endif()

set(CMAKE_PREFIX_PATH "../libuo")

set(CMAKE_C_STANDARD 11)

add_executable(smart_mirror
	src/render_instr.c
    src/smart_mirror.c
    src/smart_mirror_conf.c
	src/time_widget.c
	src/weather_widget.c)

target_include_directories("${CMAKE_PROJECT_NAME}"
    PRIVATE
        src
		../libuo/uo_sock/include)

find_library(LIBUO_SOCK 
    NAMES uo_sock
    HINTS "${CMAKE_PREFIX_PATH}/uo_sock/lib")

if(WIN32)
    target_include_directories("${CMAKE_PROJECT_NAME}"
        PRIVATE
            ./lib/SDL/include
			./lib/SDL_image/include
			./lib/SDL_ttf/include)

	find_library(LIBSDL2 SDL2
		HINTS ./lib/SDL/lib/win64)
	find_library(LIBSDL2main SDL2main
		HINTS ./lib/SDL/lib/win64)
	find_library(LIBSDL2_image SDL2_image
		HINTS ./lib/SDL_image/lib/x64)
	find_library(LIBSDL2_ttf SDL2_ttf
		HINTS ./lib/SDL_ttf/lib/x64)

	target_link_libraries(smart_mirror
		PRIVATE
			"${LIBSDL2main}")
else()
	target_include_directories("${CMAKE_PROJECT_NAME}"
        PRIVATE
            /usr/include/SDL2)

	find_library(LIBSDL2 SDL2)
	find_library(LIBSDL2_image SDL2_image)
	find_library(LIBSDL2_ttf SDL2_ttf)
endif()

target_link_libraries("${CMAKE_PROJECT_NAME}"
    PRIVATE
        "${LIBSDL2}"
		"${LIBSDL2_image}"
		"${LIBSDL2_ttf}"
		"${LIBUO_SOCK}")

if(WIN32)
    target_link_libraries("${CMAKE_PROJECT_NAME}"
        PUBLIC
            ws2_32)
endif()

set_target_properties("${CMAKE_PROJECT_NAME}"
    PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY            "${CMAKE_SOURCE_DIR}/bin/release"
        RUNTIME_OUTPUT_DIRECTORY_RELEASE    "${CMAKE_SOURCE_DIR}/bin/release"
        RUNTIME_OUTPUT_DIRECTORY_DEBUG      "${CMAKE_SOURCE_DIR}/bin/debug"
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    configure_file(conf/smart_mirror.conf
        "${CMAKE_SOURCE_DIR}/bin/debug/smart_mirror.conf" COPYONLY)
	file(COPY assets 
		DESTINATION "${CMAKE_SOURCE_DIR}/bin/debug")
else()
    configure_file(conf/smart_mirror.conf
        "${CMAKE_SOURCE_DIR}/bin/release/smart_mirror.conf" COPYONLY)
	file(COPY assets 
		DESTINATION "${CMAKE_SOURCE_DIR}/bin/release")
endif()
