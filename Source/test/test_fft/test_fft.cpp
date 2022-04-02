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

  int fft_order = log2(data.size());
  int fftSize = 1 << fft_order;
  assert(fftSize == data.size());
  std::cerr << "input size " << data.size() << ", fft order " << fft_order << std::endl;

  auto dsp = std::make_unique<WaveDsp>(10, 6, fft_order, 64, 512);

  float sr = 44100;
  float f = 440;

  dsp->forward(data.data());

  for (int i = 0; i < fftSize / 2; i++) {
    std::cout << data[i] << std::endl;
  }
}