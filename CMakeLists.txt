cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(PROJECT_NAME EditorLineLimitBypass)
project(${PROJECT_NAME})

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

# this is so stupid
set(CMAKE_SIZEOF_VOID_P 4)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  # ensure 32 bit on clang
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target i386-pc-windows-msvc")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target i386-pc-windows-msvc")
  add_definitions("--target=i386-pc-windows-msvc")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE libraries/hackpro/include)
target_link_libraries(${PROJECT_NAME} ${CMAKE_SOURCE_DIR}/libraries/hackpro/bin/hackpro.lib)
target_link_options(${PROJECT_NAME} PRIVATE "/DELAYLOAD:hackpro.dll")
