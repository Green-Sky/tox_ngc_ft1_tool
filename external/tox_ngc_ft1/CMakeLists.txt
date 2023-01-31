cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

add_library(tox_ngc_ft1 STATIC
	./tox_ngc_ft1/ngc_ft1.h
	./tox_ngc_ft1/ngc_ft1.cpp

	./tox_ngc_ft1/ledbat.hpp
	./tox_ngc_ft1/ledbat.cpp
)

target_include_directories(tox_ngc_ft1 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/tox_ngc_ft1")

target_compile_features(tox_ngc_ft1 PUBLIC cxx_std_17)

target_link_libraries(tox_ngc_ft1 PUBLIC toxcore tox_ngc_ext)

