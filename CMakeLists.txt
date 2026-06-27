# pongc/CMakeLists.txt
# CMake build file for PongC
# 
# Build for production:
# $ cmake -B build -DCMAKE_BUILD_TYPE=Release
#
# Build for debug:
# $ cmake -B build -DCMAKE_BUILD_TYPE=Debug
#
# More informations at README.md

cmake_minimum_required(VERSION 3.20)

project(pongc VERSION 1.4 LANGUAGES C)

add_executable(pongc)

target_compile_features(pongc PRIVATE c_std_23)

target_compile_options(pongc PRIVATE
  -Wall
  -Wextra
  $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
  $<$<CONFIG:Release>:-O3 -DNDEBUG>
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
  target_sources(pongc PRIVATE src/external/tinycthread.c)
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

target_include_directories(pongc PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_custom_target(distclean
  COMMAND ${CMAKE_COMMAND} -E rm -rf CMakeCache.txt CMakeFiles cmake_install.cmake Makefile
)
