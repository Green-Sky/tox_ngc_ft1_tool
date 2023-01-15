#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <cassert>
#include <string>

struct SHA1Digest {
	std::array<uint8_t, 20> data;

	SHA1Digest(const std::vector<uint8_t>& v) {
		assert(v.size() == data.size());
		for (size_t i = 0; i < data.size(); i++) {
			data[i] = v[i];
		}
	}
};

struct FTInfoSHA1 {
	std::string file_name;
	uint64_t file_size {0};
	static constexpr size_t chunk_size {4*1024}; // 4KiB for now
	std::vector<SHA1Digest> chunks;

	std::vector<uint8_t> toBuffer(void) const {
		std::vector<uint8_t> buffer;

		assert(!file_name.empty());
		// TODO: optimize
		for (size_t i = 0; i < 256; i++) {
			if (i < file_name.size()) {
				buffer.push_back(file_name.at(i));
			} else {
				buffer.push_back(0);
			}
		}
		assert(buffer.size() == 256);

		{ // HACK: endianess
			buffer.push_back((file_size>>(0*8)) & 0xff);
			buffer.push_back((file_size>>(1*8)) & 0xff);
			buffer.push_back((file_size>>(2*8)) & 0xff);
			buffer.push_back((file_size>>(3*8)) & 0xff);
			buffer.push_back((file_size>>(4*8)) & 0xff);
			buffer.push_back((file_size>>(5*8)) & 0xff);
			buffer.push_back((file_size>>(6*8)) & 0xff);
			buffer.push_back((file_size>>(7*8)) & 0xff);
		}
		assert(buffer.size() == 256+8);

		// chunk size?

		for (const auto& chunk : chunks) {
			for (size_t i = 0; i < chunk.data.size(); i++) {
				buffer.push_back(chunk.data[i]);
			}
		}
		assert(buffer.size() == 256+8+20*chunks.size());

		return buffer;
	}
};

