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

find_package(SFML 2.5 COMPONENTS graphics REQUIRED)
target_link_libraries(clonecraft PRIVATE sfml-graphics)

if(UNIX)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(clonecraft PRIVATE Threads::Threads)
endif()

find_package(OpenGL REQUIRED)

target_include_directories(clonecraft PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                           $<INSTALL_INTERFACE:include>)
target_link_libraries(clonecraft PUBLIC OpenGL::GL ${CMAKE_DL_LIBS})

if(MSVC)
  target_compile_options(clonecraft PRIVATE /W3)
else()
  target_compile_options(clonecraft PRIVATE -Wall -Wextra)
endif()