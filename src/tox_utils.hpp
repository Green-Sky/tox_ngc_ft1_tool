#pragma once

#include <string>
#include <vector>

std::vector<uint8_t> hex2bin(const std::string& str);
std::string bin2hex(const std::vector<uint8_t>& bin);

