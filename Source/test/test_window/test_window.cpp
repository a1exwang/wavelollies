#include "../../dsp.h"

#include <cassert>
#include <cmath>

#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <memory>

int main() {
  float v;
  std::vector<float> data;
  while (std::cin >> v) {
    data.push_back(v);
  }

  int window_order = log2(data.size());
  int window_size = 1 << window_order;
  assert(window_size == data.size());
  std::cerr << "input size " << data.size() << ", window order " << window_order << std::endl;

  auto dsp = std::make_unique<WaveDsp>(window_order, 6, window_order, 64, 512);

  float sr = 44100;
  float f = 440;

  dsp->window(data.data());

  for (int i = 0; i < window_size; i++) {
    std::cout << data[i] << std::endl;
  }
}