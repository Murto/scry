set(CMAKE_CXX_FLAGS "-O3 -flto -Wall -Wextra -Wpedantic -pedantic -Werror")
include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(test test.cpp)
