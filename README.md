# A spectrogram VST3 plugin similar to IL Wave Candy

## Dependencies
- fftw3 (tested with 3.3.10)
- tinycolormap header file

## Features
- Render with OpenGL to get a decent frame rate. Rendering code can run at least 60fps with 512x2048 resolution.
- DSP code can run 300fps with 2048 block size, 99.609375% overlap and 65536 FFT size. Un-bottleneck the rendering code.
