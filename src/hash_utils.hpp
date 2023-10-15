#pragma once

#include <cstdint>
#include <cstddef>
#include <vector>

// returns the 20bytes sha1 hash
std::vector<uint8_t> hash_sha1(const uint8_t* data, size_t size);

inline std::vector<uint8_t> hash_sha1(const char* data, size_t size) { return hash_sha1(reinterpret_cast<const uint8_t*>(data), size); }

