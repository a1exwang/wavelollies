#include "dsp.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

#include <cassert>
#include <cmath>

#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <iomanip>
#include <iostream>

WaveDsp::WaveDsp(int windowOrder, int strideOrder, int fftOrder, int nsinc,
                 int bins, float mindb, float maxdb)
    : windowOrder(windowOrder),
      strideOrder(strideOrder),
      fftOrder(fftOrder),
      nsinc(nsinc),
      bins(bins),
      mindb(mindb),
      maxdb(maxdb) {
  assert(fft_size >= window_size);
  assert(maxdb >= mindb);
#ifdef _WIN32
  char path[MAX_PATH] = { 0 };
  //auto lib = LoadLibrary("libfftw3f-3.3.10.dll");
  auto lib = LoadLibrary("libfftw3f.dll");
  if (lib) {
    pfftwf_plan_dft_r2c_1d =
        (decltype(pfftwf_plan_dft_r2c_1d))GetProcAddress(lib, "fftwf_plan_dft_r2c_1d");
    pfftwf_execute_dft_r2c =
        (decltype(pfftwf_execute_dft_r2c))GetProcAddress(lib, "fftwf_execute_dft_r2c");
  }
  GetModuleFileNameA(lib, path, MAX_PATH);
  std::cerr << "Using DLL: " << path << std::endl;
#else
  pfftwf_plan_dft_r2c_1d = (decltype(pfftwf_plan_dft_r2c_1d))dlsym(
      RTLD_DEFAULT, "fftwf_plan_dft_r2c_1d");
  pfftwf_execute_dft_r2c = (decltype(pfftwf_execute_dft_r2c))dlsym(
      RTLD_DEFAULT, "fftwf_execute_dft_r2c");

  auto lib = dlopen("libfftw3f.so", RTLD_LAZY);
  if (!pfftwf_plan_dft_r2c_1d && lib) {
    pfftwf_plan_dft_r2c_1d =
        (decltype(pfftwf_plan_dft_r2c_1d))dlsym(lib, "fftwf_plan_dft_r2c_1d");
    pfftwf_execute_dft_r2c =
        (decltype(pfftwf_execute_dft_r2c))dlsym(lib, "fftwf_execute_dft_r2c");
    std::cerr << "Use system libfftw3f.so" << std::endl;
  } else {
    std::cerr << "Use builtin fftw3f" << std::endl;
  }
#endif
  assert(pfftwf_plan_dft_r2c_1d);
  assert(pfftwf_execute_dft_r2c);

  std::vector<float> initBuffer(fft_size + 4);
  plan_ =
      pfftwf_plan_dft_r2c_1d(fft_size, initBuffer.data(),
                             (fftwf_complex *)initBuffer.data(), FFTW_MEASURE);
  assert(plan_);
  std::cerr << "init ok" << std::endl;

  stack.reserve(bins);

  init_window();
}

void WaveDsp::forward(float *data) {
  pfftwf_execute_dft_r2c(plan_, data, fft_buffer.data());

  for (int i = 0; i < fft_size / 2; i++) {
    auto &x = fft_buffer[i];
    data[i] = sqrtf(x[0] * x[0] + x[1] * x[1]);
  }
}

void WaveDsp::window(float *data) {
  for (int i = 0; i < window_size; i++) {
    data[i] *= window_data[i];
  }
}

void WaveDsp::scale(float *data, size_t size, float factor) {
  for (int i = 0; i < size; i++) {
    data[i] *= factor;
  }
}

void WaveDsp::init_window() {
  const auto N = window_size;

  // hann
  // for (int i = 0; i < window_size; i++) {
  //   auto a = sinf(PI * i / (N - 1));
  //   window_data[i] = a * a;
  // }

  // blackman-harris window
  for (int i = 0; i < window_size; i++) {
    float a0 = 0.35875f, a1 = 0.48829f, a2 = 0.14128f, a3 = 0.01168f;
    window_data[i] = a0 - a1 * cosf(2*PI*i/N) + a2*cosf(4*PI*i/N) - a3*cosf(6*PI*i/N);
  }

  // flat top window
  // for (int i = 0; i < window_size; i++) {
  //   float a0 = 0.21557897f, a1 = 0.41663158f, a2 = 0.277263158f, a3 = 0.083578947f, a4 = 0.006947368f;
  //   window_data[i] = a0 - a1 * cosf(2*PI*i/N) + a2*cosf(4*PI*i/N) - a3*cosf(6*PI*i/N) + a4*cosf(8*PI*i/N);
  // }
}
// assuming data is fft_size, output will be image_height
void WaveDsp::interpolate(float *output, const float *data, float sr, float lowfreq, float highfreq, float epsilon) {
  float k = log2(highfreq / lowfreq);
  for (auto i = 0; i < bins; ++i) {
    auto freq = lowfreq * pow(2, k * i / bins);
    double fftIndexReal = freq / sr * fft_size;
    float v = 0;
    int fftIndexL = int(fftIndexReal);
    int fftIndexR = fftIndexL + 1;
    // nearest
    if (true) {
        if (fftIndexL < fft_size / 2) {
            v = data[fftIndexL];
        }
        else {
            v = 0;
        }
    }
    else {
		// sinc interpolation
		for (int m = fftIndexL - (nsinc / 2 - 1);
			  m <= fftIndexR + (nsinc / 2 - 1); m++) {
		  if (0 <= m && m < fft_size / 2) {
			float z = fftIndexReal - m;
			if (fabsf(z) < epsilon) {
			  v += data[m];
			} else {
			  v += data[m] * sin(z * PI) / (z * PI);
			}
		  }
		}
    }
    output[i] = v < 0 ? 0 : v / 2;
  }
}

void WaveDsp::db(float *data, size_t size) {
  for (int i = 0; i < size; i++) {
    data[i] = 20 * log10(data[i]);
  }
}

void WaveDsp::clip(float *data, size_t size, float min, float max) {
  for (int i = 0; i < size; i++) {
    if (data[i] < min) {
      data[i] = min;
    } else if (data[i] > max) {
      data[i] = max;
    }
  }
}

void WaveDsp::slope(float *data, size_t size, float db_per_oct, bool is_db) {
  float lowfreq = 20;
  float hifreq = 20000;
  float anchor = sqrt(lowfreq * hifreq);

  float k = log2(hifreq / lowfreq);
  for (int i = 0; i < size; i++) {
    auto freq = lowfreq * pow(2, k * i / bins);
    float db = log2(freq / anchor) * db_per_oct;

    if (is_db) {
      data[i] += db;
    } else {
      data[i] *= pow(10, db / 10);
    }
  }
}

void WaveDsp::find_peak(float *output, const float *input, float slope) {
  std::fill(output, &output[bins], 0.0f);
  const float epsilon = slope * log2(20000 / 20) / bins;
  // std::cerr << "epsilon " << epsilon << std::endl;
  bool found = false;
  bool rising = false;
  for (int i = 1; i < bins; i++) {
    const auto prev = input[i - 1];
    const auto curr = input[i];

    // std::cerr << "curr " << i << " " << curr << std::endl;

    if (curr - prev > epsilon) {
      // rising
      stack.clear();
      rising = true;
      stack.push_back(i);
      // std::cerr << "rise" << std::endl;
    } else if (fabsf(prev - curr) <= epsilon) {
      // level
      stack.push_back(i);
      // std::cerr << "level" << std::endl;
    } else {
      // falling
      // std::cerr << "fall" << std::endl;
      if (rising) {
        while (!stack.empty()) {
          auto index = stack.back();
          output[index] = 1;
          stack.pop_back();
        }
      } else {
        stack.clear();
      }
      rising = false;
    }
  }

  if (rising) {
    while (!stack.empty()) {
      auto index = stack.back();
      output[index] = 1;
      stack.pop_back();
    }
  }
}
void WaveDsp::multiply(float *output, const float *input, size_t size) {
  for (int i = 0; i < size; i++) {
    output[i] *= input[i];
  }
}
void WaveDsp::e2e(float *output, float *input, float sr) {
  window(input);

  std::fill(fft_data.begin(), fft_data.end(), 0.0f);
  std::copy(input, input + window_size,
            fft_data.begin() + (fft_size - window_size) / 2);

  forward(fft_data.data());

  scale(fft_data.data(), fft_data.size(), 1.0f / window_size);

  interpolate(bins_data.data(), fft_data.data(), sr);

  std::vector<float> peaks(bins);
  find_peak(peaks.data(), bins_data.data(), 0.001f);

  multiply(bins_data.data(), peaks.data(), bins);

  db(bins_data.data(), bins_data.size());

  slope(bins_data.data(), bins_data.size(), -3, true);

  clip(bins_data.data(), bins_data.size(), mindb, maxdb);

  std::copy(bins_data.begin(), bins_data.end(), output);
}

std::string WaveDsp::dump_param() const {
  std::stringstream ss;
  ss << "dump_param" << std::endl;
  ss << "  window_size: " << window_size << std::endl;
  ss << "  stride_size: " << strideSize << std::endl;
  ss << "  fft_size: " << fft_size << std::endl;
  ss << "  nsinc: " << nsinc << std::endl;
  ss << "  bins: " << bins << std::endl;
  ss << "  mindb: " << mindb << std::endl;
  ss << "  maxdb: " << maxdb << std::endl;

  return ss.str();
}