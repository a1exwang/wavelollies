#include "../dsp.h"
#include "./utils.h"

#include <cassert>
#include <cmath>

#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <memory>

int main() {
  auto [data, fft_order] = read_floats(std::cin);
  std::cerr << "input size " << data.size() << ", fft order " << fft_order << std::endl;

  auto dsp = std::make_unique<WaveDsp>(10, 6, fft_order, 64, 512);

  float sr = 44100;
  float f = 440;

  dsp->forward(data.data());

  write_floats(std::cout, data.data(), (1<<fft_order) / 2);
}