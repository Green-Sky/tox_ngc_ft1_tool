#include "./tox_client.hpp"

#include "./command_line.hpp"

#include <memory>
#include <iostream>
#include <unordered_map>

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

	while (true) {
		client.iterate();
	}

	return 0;
}

