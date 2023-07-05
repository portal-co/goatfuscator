build/build.ninja: $(shell find .)
	cmake -S. -Bbuild -GNinja

build/stamp: build/build.ninja
	ninja -C build

# -passes="bb2func,flatenning,connect,obfCon"

hello.bc: hello.c build/stamp
	clang-16 -O2 hello.c -emit-llvm -c -o hello.bc

hello.obf.bc: hello.bc
	opt-16 -load-pass-plugin=./build/libGoatf.so  -passes="function(bb2func,flattening,connect,obfCon),merge" hello.bc -o hello.obf.bc

hello.obf: hello.obf.bc
	clang-16 ./hello.obf.bc -o hello.obf