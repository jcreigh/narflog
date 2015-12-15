NarfLog
=======

A small and narftastic C++ logging library with levels, subloggers, and views.

# Building

## Prerequisites

- CMake 2.6 or newer
- GoogleTest (gtest) development libraries (optional, used for unit tests)

## Build Commands

First get a copy of the source:

	git clone https://github.com/jcreigh/narflog.git
	cd narflog
	git submodule update --init

Make a new directory to build everything in

	mkdir build
	cd build

Then generate a Makefile using CMake and perform the build:

	cmake .. && make

Or on Windows with MinGW in an MSYS shell:

	cmake -G "MSYS Makefiles" .. && make
