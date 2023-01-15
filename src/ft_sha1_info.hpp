#pragma once

#include <cstdint>
#include <array>
#include <ostream>
#include <vector>
#include <cassert>
#include <string>

struct SHA1Digest {
	std::array<uint8_t, 20> data;

	SHA1Digest(const std::vector<uint8_t>& v);

	SHA1Digest(const uint8_t* d, size_t s);

	bool operator==(const SHA1Digest& other) const { return data == other.data; }
	bool operator!=(const SHA1Digest& other) const { return data != other.data; }

	size_t size(void) const { return data.size(); }
};

std::ostream& operator<<(std::ostream& out, const SHA1Digest& v);

struct FTInfoSHA1 {
	std::string file_name;
	uint64_t file_size {0};
	static constexpr size_t chunk_size {4*1024}; // 4KiB for now
	std::vector<SHA1Digest> chunks;

	std::vector<uint8_t> toBuffer(void) const;
};

