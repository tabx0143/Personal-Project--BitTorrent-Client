BUILD_DIR = build

.PHONY: all build clean run test docker-build docker-run

all: build

build:
	cmake -S . -B $(BUILD_DIR) && cmake --build $(BUILD_DIR) -- -j

clean:
	rm -rf $(BUILD_DIR)

run: build
	./$(BUILD_DIR)/ctorrent

test: build
	./$(BUILD_DIR)/test_bencode

docker-build:
	docker build -t ctorrent:local .

docker-run:
	docker run --rm -it ctorrent:local
