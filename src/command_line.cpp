#include "./command_line.hpp"

#include <charconv>
#include <iostream>
#include <cassert>

CommandLine::CommandLine(int argc, char** argv) {
	assert(argc > 0);

	exe = argv[0];

	for (int i = 1; i < argc; i++) {
		std::string_view arg_sv{argv[i]};

#define PRINT_HELP_AND_BAIL printHelp(); _should_exit = true; return;

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
				std::cerr << "ERROR: -G missing <chat_id> parameter!\n\n";
				PRINT_HELP_AND_BAIL;
			}
			chat_id = argv[++i];
		} else if (arg_sv == "-F") {
			if (i+1 >= argc) {
				std::cerr << "ERROR: -F missing <path> parameter!\n\n";
				PRINT_HELP_AND_BAIL;
			}
			profile_path = argv[++i];
		} else if (arg_sv == "-N") {
			if (i+1 >= argc) {
				std::cerr << "ERROR: -N missing <self_name> parameter!\n\n";
				PRINT_HELP_AND_BAIL;
			}
			self_name = argv[++i];
		} else if (arg_sv == "-a") {
			if (i+1 >= argc) {
				std::cerr << "ERROR: -a missing <transfer_variant> parameter!\n\n";
				PRINT_HELP_AND_BAIL;
			}
			std::string_view tv_sv{argv[++i]};

			if (tv_sv == "id1") {
				transfer_variant = TransferE::ID;
			} else if (tv_sv == "sha1_single") {
				transfer_variant = TransferE::SHA1_SINGLE;
			} else if (tv_sv == "sha1_info") {
				transfer_variant = TransferE::SHA1_INFO;
			} else {
				std::cerr << "ERROR: invalid <transfer_variant> parameter!\n\n";
				PRINT_HELP_AND_BAIL;
			}

		} else if (arg_sv == "-f") {
			if (i+1 >= argc) {
				std::cerr << "ERROR: -f missing <path> parameter!\n\n";
				PRINT_HELP_AND_BAIL;
			}
			send_path = argv[++i];
		} else if (arg_sv == "-d") {
			if (i+1 >= argc) {
				std::cerr << "ERROR: -d missing <path> parameter!\n\n";
				PRINT_HELP_AND_BAIL;
			}
			receive_dump_dir = argv[++i];
		} else if (arg_sv == "-D") {
			if (i+1 >= argc) {
				std::cerr << "ERROR: -D missing <id/hash> parameter!\n\n";
				PRINT_HELP_AND_BAIL;
			}
			receive_id = argv[++i];
		} else if (arg_sv == "-L") {
			tox_disable_local_discovery = true;
		} else if (arg_sv == "-U") {
			tox_disable_udp = true;
		} else if (arg_sv == "-P") {
			if (i+2 >= argc) {
				std::cerr << "ERROR: -P missing <host> and/or <port> parameter!\n\n";
				PRINT_HELP_AND_BAIL;
			}

			std::string_view host_sv{argv[++i]};
			std::string_view port_sv{argv[++i]};

			proxy_host = host_sv;

			auto res = std::from_chars(port_sv.data(), port_sv.data()+port_sv.size(), proxy_port);
			if (res.ptr != port_sv.data()+port_sv.size()) {
				std::cerr << "ERROR: invalid <port>!\n\n";
				PRINT_HELP_AND_BAIL;
			}
			std::cout << "CL set proxy to " << proxy_host << proxy_port << "\n";
		} else if (arg_sv == "-p") {
			if (i+1 >= argc) {
				std::cerr << "ERROR: -p missing <port> parameter!\n\n";
				PRINT_HELP_AND_BAIL;
			}

			std::string_view port_sv{argv[++i]};
			auto res = std::from_chars(port_sv.data(), port_sv.data()+port_sv.size(), tox_port);
			if (res.ptr != port_sv.data()+port_sv.size()) {
				std::cerr << "ERROR: invalid <port>!\n\n";
				PRINT_HELP_AND_BAIL;
			}
			std::cout << "CL set tox_port to " << tox_port << "\n";
		} else {
			std::cerr << "ERROR: unknown parameter '" << arg_sv << "' !\n\n";
			PRINT_HELP_AND_BAIL;
		}
	}

	if (transfer_variant == TransferE::INVALID) {
		std::cerr << "ERROR: transfer_variant not set!\n\n";
		PRINT_HELP_AND_BAIL;
	}
}

void CommandLine::printHelp(void) {
	std::cout
		<< "~~~ HELP ~~~\n"
		<< " meta:\n"
		<< " -v version info\n"
		<< " -V verbose\n"
		<< " -h help\n"
		<< "\n"
		<< " connectivity:\n"
		<< " -G <chat_id>\n"
		<< " -F profile.tox\n"
		<< " -N <self_name> (defaults to 'tox_ngc_ft1_tool')\n"
		<< " will print friend id at startup\n"
		<< " will autoaccept any invite\n"
		<< " if no -F given, will not save profile.\n"
		<< " if profile exists load, otherwise create new\n"
		<< "\n"
		<< " transfer variant:\n"
		<< " -a id1/sha1_single/sha1_info/sha2_single/sha2_info\n"
		<< "\n"
		<< " send:\n"
		<< " -f send_this_file.zip\n"
		<< "\n"
		<< " receive:\n"
		<< " -d dump/everything/in/this/dir\n"
		<< " -D <id/hash> (what to dl)\n"
		<< "\n"
		<< "!!! ADVANCED !!!\n"
		<< " tox:\n"
		<< " -L disable local discovery\n"
		<< " -U disable udp\n"
		<< " -P proxy_host proxy_port\n"
		<< " -p tox_port (bind tox to that port)\n"
		<< "\n"
		<< " FT1:\n"
		<< " TODO\n"
		<< "\n"
		<< " transfer logic:\n"
		<< " TODO\n"
	;
}

