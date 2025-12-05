cmake_minimum_required(VERSION 3.17)
project(HashTablesOpenAddressing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main executable - runs a single trace file
add_executable(HashTablesOpenAddressing
        main.cpp
        HashTableDictionary.cpp
        HashTableDictionary.hpp
        InvertedListDictionary.hpp
        InvertedListDictionary.cpp
        SmallIntMixedOperations.cpp
        SmallIntMixedOperations.hpp
        Operations.hpp
)

# Harness executable - runs multiple trace files and outputs CSV
add_executable(harness
        harness.cpp
        HashTableDictionary.cpp
        HashTableDictionary.hpp
        InvertedListDictionary.hpp
        InvertedListDictionary.cpp
        SmallIntMixedOperations.cpp
        SmallIntMixedOperations.hpp
        Operations.hpp
        RunMetaData.hpp
        RunResults.hpp
        lru_trace_generator.cpp
)