cmake_minimum_required(VERSION 3.8)
project(dsp)

add_executable(test_fft dsp.cpp test/test_fft/test_fft.cpp)
target_link_libraries(test_fft dl)

add_executable(test_window dsp.cpp test/test_window/test_window.cpp)
target_link_libraries(test_window dl)

add_executable(test_interpolate dsp.cpp test/test_interpolate/test_interpolate.cpp)
target_link_libraries(test_interpolate dl)

add_executable(test_e2e dsp.cpp test/test_e2e.cpp)
target_link_libraries(test_e2e dl)