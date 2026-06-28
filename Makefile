SRC_DIR := src/

.PHONY: release
release:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
	cmake --build build --parallel $(nproc)

.PHONY: debug
debug:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
	cmake --build build --parallel $(nproc)

.PHONY: clean
clean:
	rm -rf build/*

.PHONY: format
format:
	find $(SRC_DIR) -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format -i {} +

.PHONY: check-format
check-format:
	find $(SRC_DIR) -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format --dry-run --Werror {} +
