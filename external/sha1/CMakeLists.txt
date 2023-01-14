cmake_minimum_required(VERSION 3.8)

project(sha1 LANGUAGES C)

add_library(sha1 STATIC
	./sha1/sha1.h
	./sha1/sha1.c
)
add_library(sha1::sha1 ALIAS sha1)

target_include_directories(sha1 PUBLIC "sha1")

