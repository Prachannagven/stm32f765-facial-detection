cmake_minimum_required(VERSION 3.23)

project(PramitDetector)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(cca_inference)

target_sources(cca_inference
	PRIVATE
		./c-inference/src/c-inference.c
)

target_include_directories(cca_inference
	PRIVATE
		outputs
)

