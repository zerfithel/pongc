cmake_minimum_required(VERSION 3.20)

project(pongc VERSION 1.2 LANGUAGES C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O2")

include_directories(${PROJECT_SOURCE_DIR}/include)

set(SOURCES
    src/main.c
    src/game.c
    src/ball.c
    src/signals.c
    src/client.c
    src/server.c
    src/utils.c
    src/shaders.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

find_package(SDL2 REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
  SDL2::SDL2 
  SDL2_ttf::SDL2_ttf 
  GLEW::GLEW 
  OpenGL::GL 
  m 
  enet
)
