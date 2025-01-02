.PHONY: build

build:
	cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug
	cd build && make -j16

	glslc Engine/shaders/main.vert -o build/vert.spv  --target-spv=spv1.0
	glslc Engine/shaders/main.frag -o build/frag.spv  --target-spv=spv1.0

	cd build && ./Game/witch
release:
	cd build && cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release
	cd build && make -j16

	glslc Engine/shaders/main.vert -o build/vert.spv  --target-spv=spv1.0
	glslc Engine/shaders/main.frag -o build/frag.spv  --target-spv=spv1.0

	cd build && ./Game/witch
