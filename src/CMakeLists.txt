cmake_minimum_required(VERSION 3.7)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

if(NOT TARGET uo::uo_conf)
    find_package(uo_conf CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_err)
    find_package(uo_err CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_httpc)
    find_package(uo_httpc CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_mem)
    find_package(uo_mem CONFIG REQUIRED)
endif()
if(NOT TARGET uo::uo_ipc)
    find_package(uo_ipc CONFIG REQUIRED)
endif()

add_executable(smart_mirror
    msg_widget.c
	render_instr.c
    smart_mirror.c
    smart_mirror_conf.c
	time_widget.c
	weather_widget.c)

target_include_directories(smart_mirror
    PRIVATE
        .)

if(WIN32)
    target_link_libraries(smart_mirror PRIVATE SDL2::SDL2main)
endif()

target_link_libraries(smart_mirror
    PRIVATE
        uo::uo_conf
        uo::uo_err
        uo::uo_httpc
        uo::uo_mem
        uo::uo_ipc
        SDL2::SDL2_image
        SDL2::SDL2_ttf
        SDL2::SDL2
        m)

target_compile_features(smart_mirror
    PUBLIC
        c_std_11)

install(DIRECTORY conf/ assets
    DESTINATION bin)

install(TARGETS smart_mirror
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)
