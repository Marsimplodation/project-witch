.PHONY: build build-fedora

build:
	#fastfetch
	cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug
	cd build && make -j16

	glslc Engine/shaders/main.vert -o build/vert.spv  --target-spv=spv1.0
	glslc Engine/shaders/main.frag -o build/frag.spv  --target-spv=spv1.0
	glslc Engine/shaders/shadow.vert -o build/shadowVert.spv  --target-spv=spv1.0
	glslc Engine/shaders/shadow.frag -o build/shadowFrag.spv  --target-spv=spv1.0
	cp -r Engine/textures build/

	cd build && ./Game/witch

build-fedora:
	#fastfetch
	cd build-fedora && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug
	cd build-fedora && make -j16

	glslc Engine/shaders/main.vert -o build-fedora/vert.spv  --target-spv=spv1.0
	glslc Engine/shaders/main.frag -o build-fedora/frag.spv  --target-spv=spv1.0
	glslc Engine/shaders/shadow.vert -o build-fedora/shadowVert.spv  --target-spv=spv1.0
	glslc Engine/shaders/shadow.frag -o build-fedora/shadowFrag.spv  --target-spv=spv1.0
	cp -r Engine/textures build-fedora/

	cd build-fedora && ./Game/witch
release:
	cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release
	cd build && make -j16

	glslc Engine/shaders/main.vert -o build/vert.spv  --target-spv=spv1.0
	glslc Engine/shaders/main.frag -o build/frag.spv  --target-spv=spv1.0

	cd build && ./Game/witch
