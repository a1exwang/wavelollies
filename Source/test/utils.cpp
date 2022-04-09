#include "utils.h"
#include <cassert>
#include <cmath>
#include <iomanip>

std::tuple<std::vector<float>, size_t> read_floats(std::istream &is) {
  float v;
  std::vector<float> data;
  while (std::cin >> v) {
    data.push_back(v);
  }

  int order = log2(data.size());
  int data_size = 1 << order;

  if (data_size != data.size()) {
    std::cerr << "input data size(" << data.size() << ") not 2^N" << std::endl;
    std::abort();
  }

  return {data, order};
}

void write_floats(std::ostream &os, float *data, size_t size) {
  for (int i = 0; i < size; i++) {
    os << std::fixed << std::setprecision(13) << data[i] << std::endl;
  }
}