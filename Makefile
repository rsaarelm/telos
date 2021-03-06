TARGET=telos

.PHONY: tools build xbuild release all run xrun clean

build: build/Makefile
	cd build/; make

build/Makefile: CMakeLists.txt
	mkdir -p build
	cd build/; cmake ..

# Tools are a separate target so that the cross-compile target can build them
# as local binaries.
tools: build/Makefile
	cd build/; make render-font && make emit-chardata && make bake-data && make build-atlas

X_OPT=-D CMAKE_TOOLCHAIN_FILE=../cmake_scripts/Toolchain-mingw32.cmake

xbuild/Makefile: CMakeLists.txt tools
	mkdir -p xbuild
	cd xbuild/; cmake $(X_OPT) ..

xbuild: xbuild/Makefile
	cd xbuild/; make

release:
	./release.sh

all: build xbuild

run: build
	./build/telos $(ARGS)

xrun: xbuild
	wine ./xbuild/telos $(ARGS)

tags:
	etags $$(find . -name "*.cpp" -or -name "*.hpp")

clean:
	rm -rf build/ xbuild/ packages/ TAGS
