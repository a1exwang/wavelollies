#include "../dsp.h"

#include <cassert>
#include <cmath>

#include <fstream>
#include <iomanip>
#include <vector>
#include <iostream>
#include <memory>

int main(int argc, char **argv) {
  float sr = atoi(argv[1]);
  int stride_order = atoi(argv[2]);
  int fft_order = atoi(argv[3]);
  int bins = atoi(argv[4]);
  int nsinc = atoi(argv[5]);
    
  float v;
  std::vector<float> data;
  while (std::cin >> v) {
    data.push_back(v);
  }

  int window_order = log2(data.size());
  int fft_size = 1 << fft_order;
  int window_size = 1 << window_order;
  assert(window_size == data.size());
  std::cerr << "input size " << data.size() << ", window order " << window_order << std::endl;

  auto dsp = std::make_unique<WaveDsp>(window_order, stride_order, fft_order, nsinc, bins);

  dsp->window(data.data());

  std::vector<float> fft_buffer(1 << fft_order, 0);
  std::copy(data.begin(), data.end(), fft_buffer.begin() + (fft_size - window_size) / 2);

  dsp->forward(fft_buffer.data());

  std::vector<float> bins_data(bins);
  dsp->interpolate(bins_data.data(), fft_buffer.data(), sr);

  for (int i = 0; i < bins; i++) {
    std::cout << bins_data[i] << std::endl;
  }
}