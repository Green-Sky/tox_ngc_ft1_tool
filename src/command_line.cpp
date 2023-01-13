#include "./command_line.hpp"

#include <iostream>
#include <cassert>

CommandLine::CommandLine(int argc, char** argv) {
	assert(argc > 0);

	exe = argv[0];

	for (int i = 1; i < argc; i++) {
		std::string_view arg_sv{argv[i]};

		if (arg_sv == "-v") {
			version = true;
			_should_exit = true;
		} else if (arg_sv == "-V") {
			verbose = true;
		} else if (arg_sv == "-h") {
			help = true;
			printHelp();
			_should_exit = true;
		} else if (arg_sv == "-G") {
			if (i+1 >= argc) {
				std::cerr << "-G missing <chat_id> parameter!\n\n";
				printHelp();
				_should_exit = true;
				return;
			}
			chat_id = argv[++i];
		} else if (arg_sv == "-F") {
			if (i+1 >= argc) {
				std::cerr << "-F missing <path> parameter!\n\n";
				printHelp();
				_should_exit = true;
				return;
			}
			profile_path = argv[++i];
		} else if (arg_sv == "-a") {
		} else if (arg_sv == "-f") {
			if (i+1 >= argc) {
				std::cerr << "-f missing <path> parameter!\n\n";
				printHelp();
				_should_exit = true;
				return;
			}
			send_path = argv[++i];
		} else if (arg_sv == "-d") {
			if (i+1 >= argc) {
				std::cerr << "-d missing <path> parameter!\n\n";
				printHelp();
				_should_exit = true;
				return;
			}
			receive_dump_dir = argv[++i];
		} else if (arg_sv == "-D") {
			if (i+1 >= argc) {
				std::cerr << "-D missing <id/hash> parameter!\n\n";
				printHelp();
				_should_exit = true;
				return;
			}
			receive_id = argv[++i];
		}
	}
}

void CommandLine::printHelp(void) {
	std::cout
		<< "meta:\n"
		<< " -v version info\n"
		<< " -V verbose\n"
		<< " -h help\n"
		<< "\n"
		<< " connectivity:\n"
		<< " -G <chat_id>\n"
		<< " -F profile.tox\n"
		<< " will print friend id at startup\n"
		<< " will autoaccept any invite\n"
		<< " if no -F give, will not save profile.\n"
		<< " if profile exists load, otherwise create new\n"
		<< "\n"
		<< " transfer variant:\n"
		<< " -a id1/sha128_single/sha128_info/sha256_single/sha256_info\n"
		<< "\n"
		<< " send:\n"
		<< " -f send_this_file.zip\n"
		<< "\n"
		<< " receive:\n"
		<< " -d dump/everything/in/this/dir\n"
		<< " -D <id/hash> (what to dl)\n"
	;
}

