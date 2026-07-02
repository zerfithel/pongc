SRC_DIR := src/
TEST_DIR := tests/*.c

.PHONY: release
release:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_TEST=ON
	cmake --build build --parallel $(nproc)

.PHONY: debug
debug:
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_TEST=ON
	cmake --build build --parallel $(nproc)

.PHONY: clean
clean:
	rm -rf build/*

.PHONY: format
format:
	find $(SRC_DIR) $(TEST_DIR) -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format -i {} +


.PHONY: check-format
check-format:
	find $(SRC_DIR) $(TEST_DIR) -type f \( -name "*.c" -o -name "*.h" \) -exec clang-format --dry-run --Werror {} +
