#pragma once
#include <cctype>
#include <cmath>
#include <vector>
#include <string>

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
          int bins, float mindb = -96, float maxdb = 0);
  ~WaveDsp() {
    // TODO: delete plan
  }

  static void scale(float *data, size_t size, float factor);

  std::string dump_param() const;

  // inplace forward, assuming data size == fft_size + 2
  // the result is magtitude
  void forward(float *data);

  // assuming data_size >= window_size
  void window(float *data);

  void interpolate(float *output, const float *data, float sr, float lowfreq = 20, float highfreq = 20000, float epsilon = 1e-4);

  static void db(float *data, size_t size);
  static void clip(float *data, size_t size, float min, float max);
  static void multiply(float *output, const float *input, size_t size);
  static void add(float *output, const float *lhs, const float *rhs, size_t size);

  void slope(float *data, size_t size, float db_per_oct, bool is_db = true);

  void find_peak(float *output, const float *input, float slope);

  // input size = bins
  // input_data is amplitude [0, 1]
  void freq_split(float *low, float *hi, const float *input);

  // input size = window_size
  // output size = bins
  void e2e(float *output, float *peaks, float *input, float sr, bool pitch_tracking);

  void amp2db(float* input, size_t size);

  int freq2bin(float freq);
  float bin2freq(int bin);

 private:
  void init_window();

 private:
  int windowOrder;
  int window_size = (1 << windowOrder);
  int strideOrder;
  int strideSize = 1 << strideOrder;
  int fftOrder;
  int fft_size = 1 << fftOrder;
  int nsinc;
  int bins;

  float mindb, maxdb;
  float minfreq = 20, maxfreq = 20000;
  // 1.0 for only peaks, 0 for only spectrogram
  float peaks_mix = 1.0f;
  float bandwidth_octs = 2.0f;
  // frequency below this will show peaks
  float split_frequency = 20000.0f;

  float octs = log2f(maxfreq / minfreq);

  std::vector<float> window_data = std::vector<float>(window_size);

  std::vector<fftwf_complex> fft_buffer = std::vector<fftwf_complex>(fft_size/2 + 2);
  std::vector<float> fft_data = std::vector<float>(fft_size);

  std::vector<size_t> stack = std::vector<size_t>(bins);
  std::vector<size_t> peaks = std::vector<size_t>(bins);
  std::vector<size_t> peaks_tmp = std::vector<size_t>(bins);
  std::vector<float> peaks_factor = std::vector<float>(bins);
  std::vector<float> lo_data = std::vector<float>(bins);
  std::vector<float> hi_data = std::vector<float>(bins);
  std::vector<float> bins_data = std::vector<float>(bins);
  std::vector<float> peaks_data = std::vector<float>(bins);

  // fftw
  fftwf_plan (*pfftwf_plan_dft_r2c_1d)(int n, float *in, fftwf_complex *out,
                                       unsigned flags) = 0;
  void (*pfftwf_execute_dft_r2c)(const fftwf_plan p, float *in, fftwf_complex *out) = 0;
  fftwf_plan plan_;
};
