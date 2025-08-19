cmake_minimum_required(VERSION 3.16)
project(enigma LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type." FORCE)
endif()

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS include/*.hpp)

add_executable(${PROJECT_NAME} ${SRC} ${HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE include)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:
    -O3 -march=native -mtune=native -DNDEBUG
    -ffunction-sections -fdata-sections
  >
  $<$<CXX_COMPILER_ID:MSVC>:
    /O2 /Ob3 /DNDEBUG /GL
  >
)

target_link_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:GNU>:
    -flto -Wl,--gc-sections
  >
  $<$<CXX_COMPILER_ID:Clang>:
    -flto=thin -Wl,--gc-sections
  >
  $<$<CXX_COMPILER_ID:MSVC>:
    /LTCG
  >
)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_msg)
if(ipo_supported)
  set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()
