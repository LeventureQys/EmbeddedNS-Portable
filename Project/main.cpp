#include "ENC_Core/AudioProcess/Api/ENC_V.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <filesystem> // C++17 or later for filesystem operations
#include <windows.h>
#include <mmreg.h> // 推荐使用这个
void S16ToFloatS16(const int16_t* src, size_t size, float* dest) {

	for (size_t i = 0; i < size; ++i)
		dest[i] = src[i];
	//dest[i] = static_cast<float>(src[i]) / 32768.0f;
}
static inline int16_t FloatS16ToS16(float v) {
	v = (std::min)(v, 32767.f);
	v = (std::max)(v, -32768.f);
	return static_cast<int16_t>(v + std::copysign(0.5f, v));
	//return static_cast<int16_t>(v * 32768.f);
}
void FloatS16ToS16(const float* src, size_t size, int16_t* dest) {
	for (size_t i = 0; i < size; ++i)
		dest[i] = FloatS16ToS16(src[i]);
}
struct WavHeader {
	char chunkId[4];         // "RIFF"
	uint32_t chunkSize;      // 文件大小 - 8
	char format[4];          // "WAVE"
	char subchunk1Id[4];     // "fmt "
	uint32_t subchunk1Size;  // 16 for PCM
	uint16_t audioFormat;    // 1 for PCM
	uint16_t numChannels;    // 声道数
	uint32_t sampleRate;     // 采样率
	uint32_t byteRate;       // 每秒字节数
	uint16_t blockAlign;     // 每个采样点的字节数
	uint16_t bitsPerSample;  // 每个采样的比特数
	char subchunk2Id[4];     // "data"
	uint32_t subchunk2Size;  // 数据大小
};


void porcess_audio_file(std::string input_filename, std::string output_filename) {
	std::ifstream input_file(input_filename, std::ios::binary);
	void* handler;
	handler = InitNSHandler(3);
	if (!input_file.is_open()) {
		std::cerr << "Error opening input file: " << input_filename << std::endl;
		return;
	}
	WavHeader header;
	input_file.read(reinterpret_cast<char*>(&header), sizeof(WavHeader));
	if (std::strncmp(header.chunkId, "RIFF", 4) != 0 ||
		std::strncmp(header.format, "WAVE", 4) != 0 ||
		std::strncmp(header.subchunk1Id, "fmt ", 4) != 0 ||
		std::strncmp(header.subchunk2Id, "data", 4) != 0) //||
		//header.audioFormat != 1 //|| // 必须是 PCM
		//header.numChannels != 2 ||
		//header.sampleRate != 48000 ||
		//header.bitsPerSample != 16) 
	{
		std::cerr << "Invalid WAV file format." << std::endl;
		return;
	}
	size_t num_samples = header.subchunk2Size / (header.bitsPerSample / 8) / header.numChannels;

	// 计算每次处理的帧长（例如，10ms）
	const int frame_size = header.sampleRate / 100; // 10ms的帧大小
	const int num_channels = header.numChannels;

	std::ofstream output_file(output_filename, std::ios::binary);
	if (!output_file.is_open()) {
		std::cerr << "Error opening output file: " << output_filename << std::endl;
		return;
	}
	// 循环处理音频数据
	size_t total_processed_samples = 0;
	std::vector<int16_t> output_pcm;
	int index = 0;
	while (total_processed_samples < num_samples) {
		// 读取一帧数据
		index++;
		if (index == 300) {
			int i = total_processed_samples;
		}

		std::vector<int16_t> pcm_data(frame_size * num_channels);
		std::vector<int16_t> pcm_output_data(frame_size * num_channels);

		size_t samples_to_read = min(frame_size, (int)(num_samples - total_processed_samples));

		// 使用 char 类型的缓冲区读取数据
		std::vector<char> buffer(samples_to_read * num_channels * sizeof(int16_t));
		input_file.read(buffer.data(), buffer.size());

		// 将 char 类型的数据转换为 int16_t 类型
		int16_t* input_i = pcm_data.data();
		for (size_t i = 0; i < buffer.size(); i += sizeof(int16_t)) {
			*input_i++ = *(int16_t*)(buffer.data() + i);
		}

		// 拿出一组float数据出来
		float input[480];
		S16ToFloatS16(pcm_data.data(), 480, input);

		float output[480] = { 0 };

		NS_Process_48kAudio(handler, (float*)input, output);

		// 处理完的数据转换成int
		int16_t* output_data = pcm_output_data.data();
		FloatS16ToS16(output, 480, output_data);

		// 输出得到的output 插入到结果中去
		output_pcm.insert(output_pcm.end(), pcm_output_data.begin(), pcm_output_data.end());
		total_processed_samples += samples_to_read;
	}
	free(handler);
	//将结果的output_pcm插入到output file中
	output_file.write(reinterpret_cast<char*>(output_pcm.data()), output_pcm.size() * sizeof(int16_t));
	// 更新输出文件的头部信息中的数据大小
	std::fstream update_output_file(output_filename, std::ios::in | std::ios::out | std::ios::binary);
	if (update_output_file.is_open()) {

		header.subchunk2Size = total_processed_samples * num_channels * sizeof(float);

		header.chunkSize = 36 + header.subchunk2Size;

		update_output_file.seekp(0, std::ios::beg);
		update_output_file.write(reinterpret_cast<char*>(&header), sizeof(WavHeader));
		update_output_file.close();
		return;
	}
}
//#include <filesystem> // C++17 or later for filesystem operations
namespace fs = std::filesystem;

int main() {
	//std::string input_filename = "D:\\temp\\48k\\4_01.wav";
	//std::string output_filename = "D:/temp/48k/output/4.wav";
	//-----------初始化handler-------------
	std::string input_folder;
	input_folder = "D:/WorkShop/Github/enc_-webrtc/Project/test_file";
	fs::path output_dir = fs::path(input_folder) / "kLtrFeatureThr0.8f";
	//fs::path output_dir = fs::path(input_folder) / "output";
	if (!fs::exists(output_dir)) {
		if (!fs::create_directory(output_dir)) {
			std::cerr << "Error creating output directory: " << output_dir << std::endl;
			return 1;
		}
	}

	// Iterate through all files in the input folder
	for (const auto& entry : fs::directory_iterator(input_folder)) {
		if (entry.is_regular_file() && entry.path().extension() == ".wav") {
			// Construct input and output file paths
			std::string input_filename = entry.path().string();
			std::string output_filename = (output_dir / entry.path().filename()).string();

			// Process the WAV file
			porcess_audio_file(input_filename, output_filename);
			
		}
	}
}