build/build.ninja: $(shell find .)
	cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_PREFIX_PATH=/usr/lib/llvm-17/ -S. -Bbuild -GNinja

build/stamp: build/build.ninja
	ninja -C build

# -passes="bb2func,flatenning,connect,obfCon"

hello.bc: hello.c build/stamp
	clang-17 -O2 hello.c -emit-llvm -c -o hello.bc

hello.obf.bc: hello.bc
	opt-17 -load-pass-plugin=./build/libGoatf.so  -passes="function(bb2func,connect,obfCon,duplicate-bb),merge" hello.bc -o hello.obf.bc

hello.obf: hello.obf.bc
	clang-17 -O3 ./hello.obf.bc -o hello.obf

run: hello.obf
	./hello.obf