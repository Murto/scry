cmake_minimum_required(VERSION 3.13)
set(CMAKE_CXX_STANDARD 17)
install(FILES initregex DESTINATION include)

project(test LANGUAGES CXX)
add_subdirectory(test)
