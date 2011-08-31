all: build test

build: clean configure compile

configure:
	node-waf configure

compile:
	node-waf build

test:
	nodeunit test/*

clean:
	node-waf clean

.PHONY: clean test build
