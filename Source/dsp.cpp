#include "dsp.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include <cassert>
#include <cmath>

#include <fstream>
#include <memory>
#include <vector>
#include <iomanip>
#include <iostream>

WaveDsp::WaveDsp(int windowOrder, int strideOrder, int fftOrder, int nsinc,
                 int imageHeight)
    : windowOrder(windowOrder),
      strideOrder(strideOrder),
      fftOrder(fftOrder),
      nsinc(nsinc),
      imageHeight(imageHeight) {
#ifdef _WIN32
  auto lib = LoadLibrary("libfftw3f.dll");
  if (lib) {
    pfftwf_plan_r2r_1d =
        (decltype(pfftwf_plan_r2r_1d))GetProcAddress(lib, "fftwf_plan_r2r_1d");
    pfftwf_plan_dft_r2c_1d =
        (decltype(pfftwf_plan_dft_r2c_1d))GetProcAddress(lib, "fftwf_plan_dft_r2c_1d");
    pfftwf_execute_r2r =
        (decltype(pfftwf_execute_r2r))GetProcAddress(lib, "fftwf_execute_r2r");
    pfftwf_execute_dft_r2c =
        (decltype(pfftwf_execute_dft_r2c))GetProcAddress(lib, "fftwf_execute_dft_r2c");
  }
#else
  pfftwf_plan_dft_r2c_1d = (decltype(pfftwf_plan_dft_r2c_1d))dlsym(
      RTLD_DEFAULT, "fftwf_plan_dft_r2c_1d");
  pfftwf_plan_r2r_1d =
      (decltype(pfftwf_plan_r2r_1d))dlsym(RTLD_DEFAULT, "fftwf_plan_r2r_1d");
  pfftwf_execute_r2r =
      (decltype(pfftwf_execute_r2r))dlsym(RTLD_DEFAULT, "fftwf_execute_r2r");
  pfftwf_execute_dft_r2c = (decltype(pfftwf_execute_dft_r2c))dlsym(
      RTLD_DEFAULT, "fftwf_execute_dft_r2c");

  auto lib = dlopen("libfftw3f.so", RTLD_LAZY);
  if (!pfftwf_plan_r2r_1d && lib) {
    pfftwf_plan_r2r_1d =
        (decltype(pfftwf_plan_r2r_1d))dlsym(lib, "fftwf_plan_r2r_1d");
    pfftwf_plan_dft_r2c_1d =
        (decltype(pfftwf_plan_dft_r2c_1d))dlsym(lib, "fftwf_plan_dft_r2c_1d");
    pfftwf_execute_r2r =
        (decltype(pfftwf_execute_r2r))dlsym(lib, "fftwf_execute_r2r");
    pfftwf_execute_dft_r2c =
        (decltype(pfftwf_execute_dft_r2c))dlsym(lib, "fftwf_execute_dft_r2c");
    std::cerr << "Use system libfftw3f.so" << std::endl;
  } else {
    std::cerr << "Use builtin fftw3f" << std::endl;
  }
#endif
  assert(pfftwf_plan_r2r_1d);
  assert(pfftwf_execute_r2r);
  assert(pfftwf_plan_dft_r2c_1d);

  std::vector<float> initBuffer(fftSize + 2);
  plan_ =
      pfftwf_plan_dft_r2c_1d(fftSize, initBuffer.data(),
                             (fftwf_complex *)initBuffer.data(), FFTW_MEASURE);
  assert(plan_);
  std::cerr << "init ok" << std::endl;

  init_window();
}

void WaveDsp::forward(float *data) {
  pfftwf_execute_dft_r2c(plan_, data, fft_buffer.data());

  for (int i = 0; i < fftSize / 2; i++) {
    auto &x = fft_buffer[i];
    data[i] = sqrtf(x[0] * x[0] + x[1] * x[1]) / fftSize;
  }
}

void WaveDsp::window(float *data) {
  for (int i = 0; i < window_size; i++) {
    data[i] *= window_data[i];
  }
}

void WaveDsp::init_window() {
  // hann window
  const auto N = window_size;
  for (int i = 0; i < window_size; i++) {
    auto a = sinf(PI * i / (N - 1));
    window_data[i] = a * a;
  }
}