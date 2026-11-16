CC = cc
CPPFLAGS = -Iinclude
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
LDLIBS = -lm

.PHONY: all test demo sanitize clean json-test
all: test demo

build:
	mkdir -p build

build/test_analysis: src/csvi_analysis.c src/csvi_json.c tests/test_analysis.c include/csvi_analysis.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/csvi_analysis.c src/csvi_json.c tests/test_analysis.c $(LDLIBS) -o $@

build/demo: src/csvi_analysis.c src/csvi_json.c examples/demo.c include/csvi_analysis.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) src/csvi_analysis.c src/csvi_json.c examples/demo.c $(LDLIBS) -o $@

test: build/test_analysis
	./build/test_analysis

demo: build/demo
	./build/demo

sanitize: | build
	$(CC) $(CPPFLAGS) -std=c11 -Wall -Wextra -Wpedantic -Werror -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer src/csvi_analysis.c src/csvi_json.c tests/test_analysis.c $(LDLIBS) -o build/test_sanitize
	./build/test_sanitize

clean:
	rm -f build/test_analysis build/demo build/test_sanitize

json-test: build/demo
	python3 tests/test_json.py
