CXX = c++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -Wpedantic
BIN_DIR ?= ./bin

.PHONY: all clean

all: $(BIN_DIR)/map_reads

$(BIN_DIR)/map_reads: src/map_reads.cpp src/tree.h src/io.h
	mkdir -p "$(BIN_DIR)"
	$(CXX) $(CXXFLAGS) src/map_reads.cpp -o "$@" -lz

clean:
	rm -f "$(BIN_DIR)/map_reads"
