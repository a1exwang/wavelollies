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
  int window_order = atoi(argv[3]);
  int fft_order = atoi(argv[4]);
  int bins = atoi(argv[5]);
  int nsinc = atoi(argv[6]);
  float mindb = atoi(argv[7]);
  float maxdb = atoi(argv[8]);
  bool pitch_tracking = std::string("true") == argv[9];
    
  float v;
  std::vector<float> data;
  while (std::cin >> v) {
    data.push_back(v);
  }

  float sum = 0;
  for (int i = 0; i < data.size(); i++) {
    sum += data[i];
  }
  float average = sum / data.size();
  float sum2 = 0;
  for (int i = 0; i < data.size(); i++) {
    sum2 += (data[i] - average) * (data[i] - average);
  }
  float std = sqrtf(sum2 / data.size());

  int fft_size = 1 << fft_order;
  int window_size = 1 << window_order;
  // must be times of window_size
  assert(data.size() % window_size == 0);
  std::cerr << "input size " << data.size() << ", mean " << average << ", std " << std << ", window order " << window_order << std::endl;

  auto dsp = std::make_unique<WaveDsp>(window_order, stride_order, fft_order, nsinc, bins, mindb, maxdb);

  std::cerr << dsp->dump_param() << std::endl;

  std::vector<float> bins_data(bins);
  for (int i = 0; i < (data.size() / window_size); i++) {
    std::cerr << "before block " << i << std::endl;
    dsp->e2e(bins_data.data(), &data.data()[window_size * i], sr, pitch_tracking);
    for (int i = 0; i < bins; i++) {
      std::cout << std::fixed << std::setprecision(13) << bins_data[i] << std::endl;
    }
  }
}