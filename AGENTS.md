# AGENTS.md — EmbeddedNS-Portable

## Build
```pwsh
# Primary (demo + library on Windows):
cd Project
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
# Output: build/bin/Release/ENC_Demo.exe + ENC_Core.dll

# Library-only (PureC):
cd PureC
mkdir build; cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

- C++17 for the demo (`main.cpp`); pure C for the library.
- `Original/` also builds with CMake but is the C++ reference — not the deliverable.
- No tests, no linter config, no CI in this repo.

## Architecture
- **`Project/ENC_Core/AudioProcess/`** — canonical C library source. Public API in `Api/ENC_V.h`.
- **`PureC/AudioProcess/`** — near-identical copy of the C library (standalone, no demo). Keep in sync with `Project/`.
- **`Original/`** — C++ WebRTC reference. Do not modify unless backporting fixes.
- **`SpilitFilter/`** — WIP standalone splitting filter with different naming conventions (`splitting_filter.c`, `sparse_fir_filter.h`).
- **`Webrtc_Source/`** — upstream webrtc-audio-processing-1.0 (Meson, not this project's build system).

## DLL export
- `ENCV` preprocessor macro controls `__declspec(dllexport)` vs `__declspec(dllimport)`.
- Defined via `add_definitions(-DENCV)` in CMake. Must be set when building the library, absent when consuming.

## Audio conventions
- **`floatS16` format**: samples are `float` in int16 range `[-32768.0, 32767.0]`, NOT normalized to `[-1, 1]`.
- **Fixed parameters**: 48 kHz, 480 samples/frame (10 ms), mono, 256-point FFT → 129 frequency bins.
- Public API entry point: `NS_Process_48kAudio(void* handler, float* input, float* output)`.
- Suppression level is set once at init via `InitNSHandler(int level)` (levels 0–3).

## Code quirks
- Comments in source files are Chinese (mostly GBK/GB2312 encoding). Some may render as garbled in UTF-8 editors.
- `ns_common.h` manually typedefs `int32_t`/`uint32_t`/`uint8_t` instead of using `<stdint.h>` — intentional for embedded portability.
- "NoiseSuppressor" is sometimes misspelled "NoiseSuppressor" (WebRTC convention, one 'p').
