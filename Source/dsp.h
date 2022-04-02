#pragma once
#include <cctype>
#include <vector>

#ifdef USE_FFTW3_HEADER
#include <fftw3.h>
#else
#define FFTW_MEASURE (0U)
typedef float fftwf_complex[2];
typedef void *fftwf_plan;
typedef int fftw_r2r_kind;
#endif

constexpr float PI = 3.14159265359f;

class WaveDsp {
 public:
  WaveDsp(int windowOrder, int strideOrder, int fftOrder, int nsinc,
          int bins);
  ~WaveDsp() {
    // TODO: delete plan
  }

  // inplace forward, assuming data size == fftSize + 2
  // the result is magtitude
  void forward(float *data);

  // assuming data_size >= window_size
  void window(float *data);

  void interpolate(float *output, const float *data, float sr, float lowfreq = 20, float highfreq = 20000, float epsilon = 1e-6);
private:
  void init_window();

 private:
  int windowOrder;
  int window_size = (1 << windowOrder);
  int strideOrder;
  int strideSize = 1 << strideOrder;
  int fftOrder;
  int fftSize = 1 << fftOrder;
  int nsinc;
  int bins;

  std::vector<fftwf_complex> fft_buffer = std::vector<fftwf_complex>(fftSize/2 + 1);
  std::vector<float> window_data = std::vector<float>(window_size);

  // fftw
  fftwf_plan (*pfftwf_plan_dft_r2c_1d)(int n, float *in, fftwf_complex *out,
                                       unsigned flags) = 0;
  fftwf_plan (*pfftwf_plan_r2r_1d)(int n, float *in, float *out,
                                  fftw_r2r_kind kind, unsigned flags) = 0;
  void (*pfftwf_execute_r2r)(const fftwf_plan p, float *in, float *out) = 0;
  void (*pfftwf_execute_dft_r2c)(const fftwf_plan p, float *in, fftwf_complex *out) = 0;
  fftwf_plan plan_;
};
