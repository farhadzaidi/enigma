cmake_minimum_required(VERSION 3.16)
project(enigma LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS include/*.hpp)

add_executable(${PROJECT_NAME} ${SRC} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE include)