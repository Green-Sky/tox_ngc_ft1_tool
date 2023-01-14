#pragma once

#include <string>

// meta:
// -v version info
// -V verbose
// -h help
//
// connectivity:
// -G <chat_id>
// -F profile.tox
// -N <self_name>
// will print friend id at startup
// will autoaccept any invite
// if no -F give, will not save profile.
// if profile exists load, otherwise create new
//
// transfer variant:
// -a id1/sha1_single/sha1_info/sha2_single/sha2_info
//
// send:
// -f send_this_file.zip
//
// receive:
// -d dump/everything/in/this/dir
// -D <id/hash> (what to dl)

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

	CommandLine(int argc, char** argv);

	void printHelp(void);

	bool _should_exit {false};
};

