#pragma once

#include <string>

enum class TransferE {
	INVALID,

	ID,
	SHA1_SINGLE, // a single chunk (no filename is transfered)
	SHA1_INFO,
	//...
};

struct CommandLine {
	std::string exe;

	// meta:
	// -v
	bool version {false};
	// -V
	bool verbose {false};
	// -h
	bool help {false};

	// connectivity:
	// -G <chat_id>
	std::string chat_id;
	// -F profile.tox
	std::string profile_path;
	// -N <self_name>
	std::string self_name {"tox_ngc_tf1_tool"};

	// transfer variant:
	// -a id1/sha1_single/sha1_info/sha2_single/sha2_info
	TransferE transfer_variant {TransferE::INVALID};

	// send:
	// -f send_this_file.zip
	std::string send_path;

	// receive:
	// -d dump/everything/in/this/dir
	std::string receive_dump_dir;
	// -D <id/hash> (what to dl)
	std::string receive_id;

	// advanced tox:
	// -L disable local discovery
	bool tox_disable_local_discovery {false};
	// -U disable udp (why?)
	bool tox_disable_udp {false};
	// -P proxy_host proxy_port
	std::string proxy_host;
	uint16_t proxy_port {0};
	// -p port (start and end is set to the same port)
	uint16_t tox_port {0};

	// ---- TODO ----

	// advanced FT1:
	// -w packet_window_size
	// TODO: all them timeouts

	// advaced dl:
	// -I max_incoming_transfers (default 16)
	// -O max_outgoing_transfers (default 4)
	// -u request chunks only from UDP-direct peers

	CommandLine(int argc, char** argv);

	void printHelp(void);

	bool _should_exit {false};
};

