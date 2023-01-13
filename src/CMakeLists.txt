cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

project(tox_ngc_ft1_tool C CXX)

########################################

add_executable(tox_ngc_ft1_tool
	./main.cpp
)

target_compile_features(tox_ngc_ft1_tool PUBLIC cxx_std_17)

target_link_libraries(tox_ngc_ft1_tool
	toxcore
	tox_ngc_ext
	tox_ngc_ft1
)

