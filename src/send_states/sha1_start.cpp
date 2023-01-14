#include "./sha1_start.hpp"

#include "../tox_client.hpp"

namespace SendStates {

SHA1Start::SHA1Start(ToxClient& tcl, const CommandLine& cl) : StateI(tcl) {
}

bool SHA1Start::iterate(void) {
	_tcl._tox;
	return false;
}

std::unique_ptr<StateI> SHA1Start::nextState(void) {
	return nullptr;
}

// sha1_info
void SHA1Start::onFT1ReceiveRequestSHA1Info(uint32_t, uint32_t, const uint8_t*, size_t) {
}

bool SHA1Start::onFT1ReceiveInitSHA1Info(uint32_t, uint32_t, const uint8_t*, size_t, const uint8_t, const size_t) {
	return false;
}

void SHA1Start::onFT1ReceiveDataSHA1Info(uint32_t, uint32_t, uint8_t, size_t, const uint8_t*, size_t) {
}

void SHA1Start::onFT1SendDataSHA1Info(uint32_t, uint32_t, uint8_t, size_t, uint8_t*, size_t) {
}

// sha1_chunk
void SHA1Start::onFT1ReceiveRequestSHA1Chunk(uint32_t, uint32_t, const uint8_t*, size_t) {
}

bool SHA1Start::onFT1ReceiveInitSHA1Chunk(uint32_t, uint32_t, const uint8_t*, size_t, const uint8_t, const size_t) {
	return false;
}

void SHA1Start::onFT1ReceiveDataSHA1Chunk(uint32_t, uint32_t, uint8_t, size_t, const uint8_t*, size_t) {
}

void SHA1Start::onFT1SendDataSHA1Chunk(uint32_t, uint32_t, uint8_t, size_t, uint8_t*, size_t) {
}

} // SendStates

