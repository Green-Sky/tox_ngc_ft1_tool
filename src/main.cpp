#include "./tox_client.hpp"

#include "./command_line.hpp"

#include <memory>
#include <iostream>
#include <unordered_map>
#include <thread>
#include <chrono>

int main(int argc, char** argv) {
	CommandLine cl(argc, argv);

	if (cl.version) {
		std::cout << "tox_ngc_ft1_tool v0.0.1\n";
	}

	if (cl._should_exit) {
		return 0;
	}

	ToxClient client(cl);

	std::cout << "tox id: " << client.getOwnAddress() << "\n";

	while (client.iterate()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	return 0;
}

