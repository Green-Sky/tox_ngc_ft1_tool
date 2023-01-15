#include "./ft_sha1_info.hpp"

#include <sodium.h>

SHA1Digest::SHA1Digest(const std::vector<uint8_t>& v) {
	assert(v.size() == data.size());
	for (size_t i = 0; i < data.size(); i++) {
		data[i] = v[i];
	}
}

SHA1Digest::SHA1Digest(const uint8_t* d, size_t s) {
	assert(s == data.size());
	for (size_t i = 0; i < data.size(); i++) {
		data[i] = d[i];
	}
}

std::ostream& operator<<(std::ostream& out, const SHA1Digest& v) {
	std::string str{};
	str.resize(v.size()*2, '?');

	// HECK, std is 1 larger than size returns ('\0')
	sodium_bin2hex(str.data(), str.size()+1, v.data.data(), v.data.size());

	out << str;

	return out;
}

std::vector<uint8_t> FTInfoSHA1::toBuffer(void) const {
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

