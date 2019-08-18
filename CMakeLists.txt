cmake_minimum_required(VERSION 3.12)
project(CloneCraft)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

file(GLOB_RECURSE sources Src/*.cpp Src/*.c Src/*.h Src/*.hpp)
file(GLOB_RECURSE external	External/*.cpp External/*.c External/*.h External/*.hpp)
file(GLOB_RECURSE data data/*)

add_executable(clonecraft ${sources} ${external} ${data})

target_include_directories(clonecraft PUBLIC Src External)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

find_package(glm CONFIG REQUIRED)
target_link_libraries(clonecraft PRIVATE glm)

find_package(SFML 2.5 COMPONENTS graphics REQUIRED)
target_link_libraries(clonecraft PRIVATE sfml-graphics)