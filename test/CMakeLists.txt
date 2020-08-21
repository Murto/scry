set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")
include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(test test.cpp)
