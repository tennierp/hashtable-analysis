# Compiler and flags
CXX     := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -g

# Directories
TRACE_DIR := traceFiles
RESULTS_DIR := results

# Executables
TRACE_GEN := lru_trace_generator
HARNESS := harness

# Source files
TRACE_GEN_SRC := lru_trace_generator.cpp
HARNESS_SRC := main.cpp HashTableDictionary.cpp InvertedListDictionary.cpp SmallIntMixedOperations.cpp

# Default target
all: $(TRACE_GEN) $(HARNESS)

# Build trace generator
$(TRACE_GEN): $(TRACE_GEN_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build harness
$(HARNESS): $(HARNESS_SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Run trace generator to produce all trace files
generate-traces: $(TRACE_GEN)
	mkdir -p $(TRACE_DIR)
	./$(TRACE_GEN)

# Run harness on ALL trace files
run-harness: $(HARNESS)
	mkdir -p $(RESULTS_DIR)
	for f in $(TRACE_DIR)/*.trace; do \
		echo "Running harness on $$f"; \
		out_name=$$(basename $$f .trace).csv; \
		./$(HARNESS) $$f > $(RESULTS_DIR)/$$out_name; \
	done

# Everything in one-shot
run-all: generate-traces run-harness

# Clean build files
clean:
	rm -f $(TRACE_GEN) $(HARNESS)
	rm -f *.o
