#pragma once

#include "./command_line.hpp"

#include <tox/tox.h>
#include <ngc_ext.h>
#include <ngc_ft1.h>

#include <string>

struct ToxClient final {
	ToxClient(const CommandLine& cl);

	void iterate(void);

	void setToxProfilePath(const std::string& new_path) { _tox_profile_path = new_path; }

	std::string getOwnAddress(void) const;

	private:
		void saveToxProfile(void);

	private:
		Tox* _tox {nullptr};
		NGC_EXT_CTX* _ext_ctx {nullptr};
		NGC_FT1* _ft1_ctx {nullptr};

		std::string _tox_profile_path;
		bool _tox_profile_dirty {false}; // set in callbacks
};

