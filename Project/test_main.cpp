#include "ENC_Core/AudioProcess/Api/ENC_V.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <chrono>
#include <cmath>
#include <numeric>
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")

namespace fs = std::filesystem;

struct WavHeader {
    char chunkId[4];
    uint32_t chunkSize;
    char format[4];
    char subchunk1Id[4];
    uint32_t subchunk1Size;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char subchunk2Id[4];
    uint32_t subchunk2Size;
};

struct TestResult {
    std::string filename;
    double duration_sec;
    double process_time_ms;
    double input_rms;
    double output_rms;
    double energy_reduction_db;
    size_t total_frames;
    size_t peak_memory_kb;
    bool success;
};

static size_t GetPeakMemoryKB() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.PeakWorkingSetSize / 1024;
    }
    return 0;
}

static size_t GetCurrentMemoryKB() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize / 1024;
    }
    return 0;
}

static inline int16_t FloatS16ToS16(float v) {
    v = (std::min)(v, 32767.f);
    v = (std::max)(v, -32768.f);
    return static_cast<int16_t>(v + std::copysign(0.5f, v));
}

TestResult process_audio_file(const std::string& input_filename, const std::string& output_filename, int ns_level) {
    TestResult result;
    result.filename = fs::path(input_filename).filename().string();
    result.success = false;
    result.input_rms = 0;
    result.output_rms = 0;
    result.energy_reduction_db = 0;
    result.process_time_ms = 0;
    result.duration_sec = 0;
    result.total_frames = 0;

    std::ifstream input_file(input_filename, std::ios::binary);
    if (!input_file.is_open()) {
        std::cerr << "[ERROR] Cannot open: " << input_filename << std::endl;
        return result;
    }

    WavHeader header;
    input_file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));
    if (std::strncmp(header.chunkId, "RIFF", 4) != 0 ||
        std::strncmp(header.format, "WAVE", 4) != 0) {
        std::cerr << "[ERROR] Invalid WAV: " << input_filename << std::endl;
        return result;
    }

    if (header.sampleRate != 48000) {
        std::cerr << "[SKIP] Not 48kHz: " << input_filename << " (" << header.sampleRate << " Hz)" << std::endl;
        return result;
    }

    size_t num_samples = header.subchunk2Size / (header.bitsPerSample / 8) / header.numChannels;
    const int frame_size = 480; // 10ms @ 48kHz
    result.duration_sec = (double)num_samples / 48000.0;

    // Initialize NS handler
    void* handler = InitNSHandler(ns_level);
    if (!handler) {
        std::cerr << "[ERROR] InitNSHandler failed" << std::endl;
        return result;
    }

    std::ofstream output_file(output_filename, std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "[ERROR] Cannot create output: " << output_filename << std::endl;
        free(handler);
        return result;
    }

    // Write placeholder header
    output_file.write(reinterpret_cast<char*>(&header), sizeof(WavHeader));

    double sum_input_sq = 0.0;
    double sum_output_sq = 0.0;
    size_t total_processed = 0;
    size_t frame_count = 0;

    result.peak_memory_kb = 0;

    auto start_time = std::chrono::high_resolution_clock::now();

    while (total_processed < num_samples) {
        size_t samples_to_read = (std::min)((size_t)frame_size, num_samples - total_processed);

        std::vector<int16_t> pcm_data(samples_to_read);
        input_file.read(reinterpret_cast<char*>(pcm_data.data()), samples_to_read * sizeof(int16_t));

        // Pad to 480 if last frame is shorter
        float input[480] = {0};
        for (size_t i = 0; i < samples_to_read; ++i) {
            input[i] = (float)pcm_data[i];
        }

        float output[480] = {0};
        NS_Process_48kAudio(handler, input, output);

        // Accumulate energy statistics
        for (size_t i = 0; i < samples_to_read; ++i) {
            sum_input_sq += (double)input[i] * (double)input[i];
            sum_output_sq += (double)output[i] * (double)output[i];
        }

        // Convert output to int16
        std::vector<int16_t> out_pcm(samples_to_read);
        for (size_t i = 0; i < samples_to_read; ++i) {
            out_pcm[i] = FloatS16ToS16(output[i]);
        }
        output_file.write(reinterpret_cast<char*>(out_pcm.data()), samples_to_read * sizeof(int16_t));

        total_processed += samples_to_read;
        frame_count++;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double elapsed_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    free(handler);
    output_file.close();

    // Update WAV header
    std::fstream update_file(output_filename, std::ios::in | std::ios::out | std::ios::binary);
    if (update_file.is_open()) {
        header.subchunk2Size = (uint32_t)(total_processed * sizeof(int16_t));
        header.chunkSize = 36 + header.subchunk2Size;
        update_file.seekp(0, std::ios::beg);
        update_file.write(reinterpret_cast<char*>(&header), sizeof(WavHeader));
        update_file.close();
    }

    // Compute statistics
    result.success = true;
    result.total_frames = frame_count;
    result.process_time_ms = elapsed_ms;
    result.peak_memory_kb = GetPeakMemoryKB();
    result.input_rms = std::sqrt(sum_input_sq / total_processed);
    result.output_rms = std::sqrt(sum_output_sq / total_processed);

    if (result.output_rms > 0.001) {
        result.energy_reduction_db = 20.0 * std::log10(result.input_rms / result.output_rms);
    } else {
        result.energy_reduction_db = 99.0; // effectively silent
    }

    return result;
}

int main() {
    std::string input_folder = "D:/WorkShop/EmbeddedNS-Portable/Test_Audio/AudioSample-48000hz";
    std::string output_folder = "D:/WorkShop/EmbeddedNS-Portable/Test_Audio/AudioSample-48000hz/output_ns";

    int ns_level = 3; // -21dB strongest suppression

    std::cout << "========================================" << std::endl;
    std::cout << "  ENC Noise Suppressor Test" << std::endl;
    std::cout << "  NS Level: " << ns_level << " (-21dB)" << std::endl;
    std::cout << "  Input:  " << input_folder << std::endl;
    std::cout << "  Output: " << output_folder << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\n[Memory] Process memory before test: " << GetCurrentMemoryKB() << " KB" << std::endl;

    if (!fs::exists(output_folder)) {
        fs::create_directories(output_folder);
    }

    std::vector<TestResult> results;
    int file_count = 0;

    for (const auto& entry : fs::directory_iterator(input_folder)) {
        if (entry.is_regular_file() && entry.path().extension() == ".wav") {
            file_count++;
            std::string input_path = entry.path().string();
            std::string output_path = (fs::path(output_folder) / entry.path().filename()).string();

            std::cout << "\n[" << file_count << "] Processing: " << entry.path().filename().string() << std::endl;

            TestResult r = process_audio_file(input_path, output_path, ns_level);
            if (r.success) {
                std::cout << "    Duration: " << r.duration_sec << " s | Frames: " << r.total_frames << std::endl;
                std::cout << "    Process Time: " << r.process_time_ms << " ms" << std::endl;
                std::cout << "    Input RMS: " << r.input_rms << " | Output RMS: " << r.output_rms << std::endl;
                std::cout << "    Energy Reduction: " << r.energy_reduction_db << " dB" << std::endl;
                std::cout << "    RTF: " << (r.process_time_ms / (r.duration_sec * 1000.0)) << std::endl;
                std::cout << "    Peak Memory: " << r.peak_memory_kb << " KB" << std::endl;
                results.push_back(r);
            }
        }
    }

    // Summary
    std::cout << "\n\n========== SUMMARY ==========" << std::endl;
    std::cout << "Files processed: " << results.size() << "/" << file_count << std::endl;

    if (!results.empty()) {
        double total_duration = 0, total_process_time = 0;
        double avg_reduction = 0;
        for (auto& r : results) {
            total_duration += r.duration_sec;
            total_process_time += r.process_time_ms;
            avg_reduction += r.energy_reduction_db;
        }
        avg_reduction /= results.size();
        double overall_rtf = total_process_time / (total_duration * 1000.0);

        std::cout << "Total audio duration: " << total_duration << " s" << std::endl;
        std::cout << "Total process time:   " << total_process_time << " ms" << std::endl;
        std::cout << "Overall RTF:          " << overall_rtf << std::endl;
        std::cout << "Avg energy reduction: " << avg_reduction << " dB" << std::endl;
        std::cout << "Peak process memory:  " << GetPeakMemoryKB() << " KB" << std::endl;
        std::cout << "Current memory:       " << GetCurrentMemoryKB() << " KB" << std::endl;
    }

    std::cout << "\n[DONE] Output files saved to: " << output_folder << std::endl;
    return 0;
}
