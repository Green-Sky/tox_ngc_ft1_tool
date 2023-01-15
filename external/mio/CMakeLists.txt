cmake_minimum_required(VERSION 3.8)

project(mio VERSION 1.1.0 LANGUAGES C CXX)

add_library(mio INTERFACE)
add_library(mio::mio ALIAS mio)

target_compile_features(mio INTERFACE cxx_std_11)

target_compile_definitions(mio INTERFACE WIN32_LEAN_AND_MEAN NOMINMAX)

target_include_directories(mio INTERFACE "single_include")

