#include "./hash_utils.hpp"

#include <sha1.h>

// returns the 20bytes sha1 hash
std::vector<uint8_t> hash_sha1(const uint8_t* data, size_t size) {
	SHA1_CTX ctx;
	SHA1Init(&ctx);

	{ // lib only takes uint32_t sizes, so chunk it
		constexpr size_t hash_block_size {0xffffffff};
		size_t i = 0;
		for (; i + hash_block_size < size; i += hash_block_size) {
			SHA1Update(&ctx, reinterpret_cast<const uint8_t*>(data) + i, hash_block_size);
		}

		if (i < size) {
			SHA1Update(&ctx, reinterpret_cast<const uint8_t*>(data) + i, size - i);
		}
	}

	std::vector<uint8_t> sha1_hash(20);
	SHA1Final(sha1_hash.data(), &ctx);
	return sha1_hash;
}

