
all: build/release/rjl build/debug/rjl test

build/release/rjl: rjl.cpp
	mkdir -p build/release
	g++ -Wall -O -o $@ $<

build/debug/rjl: rjl.cpp
	mkdir -p build/debug
	g++ -Wall -ggdb -o $@ $<

test:
	build/release/rjl

clean:
	rm -rf build

.PHONY: test clean