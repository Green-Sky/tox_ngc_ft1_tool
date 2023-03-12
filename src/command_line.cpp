#include "./command_line.hpp"

#include <charconv>
#include <iostream>
#include <cassert>
#include <type_traits>

struct CLParser {
	const size_t argc;
	const char*const* argv;
	size_t& i;

	bool error {false};

bool parseFlag(std::string_view arg_sv, bool& flag) {
	std::string_view arg0_sv{argv[i]};

	if (arg0_sv != arg_sv) {
		return false;
	}

	flag = true;

	return true;
}

template<typename FN, typename Arg0, typename ...Args>
static void visit(FN&& fn, Arg0&& arg0, Args&& ...args) {
	fn(arg0);
	(fn(args),...);
}

template<typename Arg0, typename ...Args>
bool parseParam(std::string_view arg_sv, Arg0& arg0, Args& ...args) {
	std::string_view arg0_sv{argv[i]};

	if (arg0_sv != arg_sv) {
		return false;
	}

	if (argc < 1+1+sizeof...(args)) {
		std::cerr << "ERROR: " << arg_sv << " not enough parameters!\n\n";
		error = true;
		return true;
	}

	visit([this](auto& arg) {
		if (error) return;

		std::string_view argX_sv{argv[++i]};

		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, std::string>) {
			arg = argX_sv;
		} else if constexpr (std::is_integral_v<T>) {
			auto res = std::from_chars(argX_sv.data(), argX_sv.data()+argX_sv.size(), arg);
			if (res.ptr != argX_sv.data() + argX_sv.size()) {
				std::cerr << "ERROR: invalid parameter!\n\n";
				error = true;
				//PRINT_HELP_AND_BAIL;
			}
		} else if constexpr (std::is_same_v<T, float>) {
			// HACK: wait for more charconv <.<
			std::string tmp_str {argX_sv};
			arg = std::stof(tmp_str);
		} else {
			assert(false && "invalid parameter type");
		}
	}, arg0, args...);

	return true;
}

};

CommandLine::CommandLine(int argc, char** argv) {
	assert(argc > 0);

	exe = argv[0];

	for (size_t i = 1; i < size_t(argc); i++) {
		CLParser parser{size_t(argc), argv, i};
		std::string_view arg_sv{argv[i]}; // alt

#define PRINT_HELP_AND_BAIL printHelp(); _should_exit = true; return;

		if (parser.parseFlag("-v", version)) {
			_should_exit = true;
		} else if (parser.parseFlag("-V", verbose)) {
		} else if (arg_sv == "-h") {
			help = true;
			printHelp();
			_should_exit = true;
		} else if (parser.parseParam("-G", chat_id)) {
		} else if (parser.parseParam("-F", profile_path)) {
		} else if (parser.parseParam("-N", self_name)) {
		} else if (std::string tv; parser.parseParam("-a", tv)) {
			if (tv == "id1") {
				transfer_variant = TransferE::ID;
			} else if (tv == "sha1_single") {
				transfer_variant = TransferE::SHA1_SINGLE;
			} else if (tv == "sha1_info") {
				transfer_variant = TransferE::SHA1_INFO;
			} else {
				std::cerr << "ERROR: invalid <transfer_variant> parameter!\n\n";
				PRINT_HELP_AND_BAIL;
			}
		} else if (parser.parseParam("-f", send_path)) {
		} else if (parser.parseParam("-d", receive_dump_dir)) {
			std::cout << "CL going to write to '" << receive_dump_dir << "'\n";
		} else if (parser.parseParam("-D", receive_id)) {
		} else if (parser.parseFlag("-L", tox_disable_local_discovery)) {
			std::cout << "CL disabled local discovery\n";
		} else if (parser.parseFlag("-U", tox_disable_udp)) {
			std::cout << "CL disabled udp\n";
		} else if (parser.parseParam("-P", proxy_host, proxy_port)) {
			std::cout << "CL set proxy to " << proxy_host << proxy_port << "\n";
		} else if (parser.parseParam("-p", tox_port)) {
			std::cout << "CL set tox_port to " << tox_port << "\n";
		} else if (parser.parseParam("--ft_ack_per_packet", ft_acks_per_packet)) {
		} else if (parser.parseParam("--ft_init_retry_timeout_after", ft_init_retry_timeout_after)) {
		} else if (parser.parseParam("--ft_sending_give_up_after", ft_sending_give_up_after)) {
		} else if (parser.parseParam("-I", max_incoming_transfers)) {
		} else if (parser.parseParam("-O", max_outgoing_transfers)) {
		} else {
			std::cerr << "ERROR: unknown parameter '" << arg_sv << "' !\n\n";
			PRINT_HELP_AND_BAIL;
		}

		if (parser.error) {
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
		<< " --ft_ack_per_packet\n"
		<< " --ft_init_retry_timeout_after\n"
		<< " --ft_sending_give_up_after\n"
		<< "\n"
		<< " transfer logic:\n"
		<< " -I <max_incoming_transfers>\n"
		<< " -O <max_outgoing_transfers>\n"
	;
}

