all: test

build: clean configure compile

configure:
	node-waf configure

compile:
	node-waf build

test: build
	node_modules/nodeunit/bin/nodeunit test/*

clean:
	node-waf clean


.PHONY: clean test build
