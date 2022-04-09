cmake_minimum_required(VERSION 3.8)
project(dsp)

set(CMAKE_CXX_STANDARD 17)
if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    link_libraries(dl)
endif ()

add_library(test_utils test/utils.cpp)
link_libraries(test_utils)

add_executable(test_fft dsp.cpp test/test_fft.cpp)
add_executable(test_window dsp.cpp test/test_window.cpp)
add_executable(test_interpolate dsp.cpp test/test_interpolate.cpp)
add_executable(test_e2e dsp.cpp test/test_e2e.cpp)
