#include "./tox_utils.hpp"

#include <sodium.h>

std::vector<uint8_t> hex2bin(const std::string& str) {
	std::vector<uint8_t> bin{};
	bin.resize(str.size()/2, 0);

	sodium_hex2bin(bin.data(), bin.size(), str.c_str(), str.length(), nullptr, nullptr, nullptr);

	return bin;
}

std::string bin2hex(const std::vector<uint8_t>& bin) {
	std::string str{};
	str.resize(bin.size()*2, '?');

	// HECK, std is 1 larger than size returns ('\0')
	sodium_bin2hex(str.data(), str.size()+1, bin.data(), bin.size());

	return str;
}

