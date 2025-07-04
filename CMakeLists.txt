cmake_minimum_required(VERSION "4.0")

project("cvtxt")

file(GLOB_RECURSE SRCS src/*.cpp src/*.hpp src/*.c src/*.h)


add_executable(${PROJECT_NAME} ${SRCS})


