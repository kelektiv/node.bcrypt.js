TESTS = test/*.js

all: test

build: clean configure compile

configure:
	node-waf configure

compile:
	node-waf build

test: build
	@./node_modules/nodeunit/bin/nodeunit \
		$(TESTS)

clean:
	rm -f bcrypt_lib.node
	rm -Rf build


.PHONY: clean test build
