cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

add_library(tox_ngc_ext STATIC
	./tox_ngc_ext/ngc_ext.h
	./tox_ngc_ext/ngc_ext.hpp
	./tox_ngc_ext/ngc_ext.cpp
)

target_include_directories(tox_ngc_ext PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/tox_ngc_ext")

target_compile_features(tox_ngc_ext PUBLIC cxx_std_17)

target_link_libraries(tox_ngc_ext PUBLIC toxcore)

