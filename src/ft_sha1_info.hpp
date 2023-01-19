#pragma once

#include <cstddef>
#include <cstdint>
#include <array>
#include <ostream>
#include <vector>
#include <cassert>
#include <string>

struct SHA1Digest {
	std::array<uint8_t, 20> data;

	SHA1Digest(void) = default;
	SHA1Digest(const std::vector<uint8_t>& v);
	SHA1Digest(const uint8_t* d, size_t s);

	bool operator==(const SHA1Digest& other) const { return data == other.data; }
	bool operator!=(const SHA1Digest& other) const { return data != other.data; }

	size_t size(void) const { return data.size(); }
};

std::ostream& operator<<(std::ostream& out, const SHA1Digest& v);

namespace std { // inject
	template<> struct hash<SHA1Digest> {
		std::size_t operator()(const SHA1Digest& h) const noexcept {
			return
				size_t(h.data[0]) << (0*8) |
				size_t(h.data[1]) << (1*8) |
				size_t(h.data[2]) << (2*8) |
				size_t(h.data[3]) << (3*8) |
				size_t(h.data[4]) << (4*8) |
				size_t(h.data[5]) << (5*8) |
				size_t(h.data[6]) << (6*8) |
				size_t(h.data[7]) << (7*8)
			;
		}
	};
} // std

struct FTInfoSHA1 {
	std::string file_name;
	uint64_t file_size {0};
	uint32_t chunk_size {128*1024}; // 128KiB for now
	std::vector<SHA1Digest> chunks;

	std::vector<uint8_t> toBuffer(void) const;
	void fromBuffer(const std::vector<uint8_t>& buffer);
};
std::ostream& operator<<(std::ostream& out, const FTInfoSHA1& v);

// TODO: use
struct FTInfoSHA1v2 {
	std::vector<std::string> file_names;
	uint64_t file_size {0};
	uint32_t chunk_size {128*1024}; // 128KiB for now
	std::vector<SHA1Digest> chunks;

	std::vector<uint8_t> toBuffer(void) const;
	void fromBuffer(const std::vector<uint8_t>& buffer);
};
std::ostream& operator<<(std::ostream& out, const FTInfoSHA1& v);

