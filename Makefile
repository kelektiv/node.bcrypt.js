TESTS = test/*.js

all: test

build: clean configure compile

configure:
	node-gyp configure

compile: configure
	node-gyp build
	npm install .

test: build
	@./node_modules/nodeunit/bin/nodeunit \
		$(TESTS)

clean:
	node-gyp clean


.PHONY: clean test build
