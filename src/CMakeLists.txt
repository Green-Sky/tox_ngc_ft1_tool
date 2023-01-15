cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

project(tox_ngc_ft1_tool C CXX)

########################################

add_executable(tox_ngc_ft1_tool
	./main.cpp

	./command_line.hpp
	./command_line.cpp

	./tox_utils.hpp
	./tox_utils.cpp

	./hash_utils.hpp
	./hash_utils.cpp

	./tox_callbacks.hpp
	./tox_callbacks.cpp

	./tox_client.hpp
	./tox_client.cpp

	./state.hpp

	./states/send_start_sha1.hpp
	./states/send_start_sha1.cpp

	./states/sha1.hpp
	./states/sha1.cpp
)

target_compile_features(tox_ngc_ft1_tool PUBLIC cxx_std_17)

target_link_libraries(tox_ngc_ft1_tool
	toxcore
	tox_ngc_ext
	tox_ngc_ft1

	mio::mio
	sha1::sha1
)

