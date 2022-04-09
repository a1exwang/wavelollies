#pragma once

#include <iostream>
#include <tuple>
#include <vector>

std::tuple<std::vector<float>, size_t> read_floats(std::istream &is);
void write_floats(std::ostream &os, float *data, size_t size);