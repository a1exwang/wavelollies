#include "../../dsp.h"

#include <cassert>
#include <cmath>

#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <memory>

int main(int argc, char **argv) {
  float sr = atof(argv[1]);
  int bins = atoi(argv[2]);
    
  float v;
  std::vector<float> data;
  while (std::cin >> v) {
    data.push_back(v);
  }

  int fft_order = log2(data.size());
  int fftSize = 1 << fft_order;
  assert(fftSize == data.size());
  std::cerr << "input size " << data.size() << ", fft order " << fft_order << std::endl;

  auto dsp = std::make_unique<WaveDsp>(10, 6, fft_order, 32, bins);

  std::vector<float> image_col(bins);
  dsp->interpolate(image_col.data(), data.data(), sr);

  for (int i = 0; i < bins; i++) {
    std::cout << image_col[i] << std::endl;
  }
}