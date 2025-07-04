cmake_minimum_required(VERSION "4.0")

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(cvtxt VERSION 0.1)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL3 REQUIRED)

add_compile_options(-Wall -Wextra -Werror)

file(GLOB_RECURSE SRCS src/*.cpp src/*.hpp src/*.c src/*.h)

add_executable(${PROJECT_NAME} ${SRCS})

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED True)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${PROJECT_NAME} PRIVATE -DDW_RELEASE=0 -D_DEBUG)
else()
  add_compile_options(-O4)
  target_compile_definitions(${PROJECT_NAME} PRIVATE -DDW_RELEASE=1)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
  ${OPENGL_LIBRARIES}
  ${GLEW_LIBRARIES}
  ${SDL3_LIBRARIES}
)
