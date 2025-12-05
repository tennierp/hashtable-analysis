//
// RunMetaData.hpp
// Stores metadata about a trace file for the hash table analysis
//

#ifndef HASH_TABLE_ANALYSIS_RUNMETADATA_HPP
#define HASH_TABLE_ANALYSIS_RUNMETADATA_HPP

#include <string>

struct RunMetaData {
    // dataset metadata
    std::string profile;      // e.g., "lru_profile"
    std::size_t N = 0;        // problem size (cache capacity)
    int seed = 0;             // RNG seed used to generate the trace
};

#endif //HASH_TABLE_ANALYSIS_RUNMETADATA_HPP