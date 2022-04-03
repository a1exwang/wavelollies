cmake_minimum_required(VERSION 3.8)
project(dsp)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    link_libraries(dl)
endif ()

add_executable(test_fft dsp.cpp test/test_fft/test_fft.cpp)

add_executable(test_window dsp.cpp test/test_window/test_window.cpp)

add_executable(test_interpolate dsp.cpp test/test_interpolate/test_interpolate.cpp)

add_executable(test_e2e dsp.cpp test/test_e2e.cpp)
