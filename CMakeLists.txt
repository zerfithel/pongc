cmake_minimum_required(VERSION 3.20)

project(pongc VERSION 1.4 LANGUAGES C)

add_executable(pongc)

target_compile_features(pongc PRIVATE c_std_23)

target_compile_options(pongc PRIVATE
  -Wall
  -Wextra
  -Wpedantic
  -Wconversion
  -Wsign-conversion
  -Wshadow
  -Wcast-align
  -Wcast-qual
  -Wformat=2
  -Wundef
  -Wdouble-promotion
  -Wnull-dereference
  -Wimplicit-fallthrough
  -Wstrict-prototypes
  -Wmissing-prototypes
  -Wold-style-definition
  -Werror

  $<$<CONFIG:Debug>:-O0>
  $<$<CONFIG:Debug>:-g3>
  $<$<CONFIG:Debug>:-DDEBUG>
  $<$<CONFIG:Debug>:-fsanitize=address,undefined,leak,pointer-compare,pointer-subtract>
  $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
  $<$<CONFIG:Debug>:-fno-common>

  $<$<CONFIG:Release>:-O3>
  $<$<CONFIG:Release>:-DNDEBUG>
)

target_link_options(pongc PRIVATE
  $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

target_sources(pongc PRIVATE
  src/main.c
  src/game.c
  src/ball.c
  src/signals.c
  src/client.c
  src/server.c
  src/utils.c
  src/shaders.c
  src/cmdline.c
)

if(WIN32)
  target_sources(pongc PRIVATE
    src/external/tinycthread.c
  )
endif()

find_package(SDL2 REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(pongc PRIVATE
  SDL2::SDL2
  GLEW::GLEW
  OpenGL::GL
  m
  enet
)

add_custom_target(distclean
  COMMAND ${CMAKE_COMMAND} -E rm -rf
    CMakeCache.txt
    CMakeFiles
    cmake_install.cmake
    Makefile
)
