//
// RunResults.hpp
// Stores results from a single harness run (timing + hash table stats)
//

#pragma once
#include <string>
#include <cstdint>
#include <sstream>
#include "RunMetaData.hpp"

struct RunResult {
    // identifiers
    std::string impl;         // "hash_map_single" or "hash_map_double"
    std::string trace_path;   // e.g., "thisOnesMine_lru_profile_N_1024_S_23.trace"
    RunMetaData run_meta_data;

    explicit RunResult(const RunMetaData& meta_data) : run_meta_data(meta_data) {}

    // timing
    std::int64_t elapsed_ns = 0;   // total replay time (nanoseconds)

    // operation counts
    long inserts = 0;     // 'I' operations
    long erases = 0;      // 'E' operations

    // hash table statistics (from csvStats())
    std::string hash_table_stats;  // the raw CSV stats from HashTableDictionary::csvStats()

    // convenience
    long total_ops() const {
        return inserts + erases;
    }

    double elapsed_ms() const {
        return static_cast<double>(elapsed_ns) / 1e6;
    }

    // CSV helpers
    static std::string csv_header() {
        return "impl,profile,trace_path,N,seed,elapsed_ms,ops_total,inserts,erases," +
               std::string("table_size,active,available,tombstones,total_probes,") +
               std::string("inserts_ht,deletes_ht,lookups,full_scans,compactions,max_in_table,") +
               std::string("available_pct,load_factor_pct,eff_load_factor_pct,") +
               std::string("tombstones_pct,average_probes,probe_type,compaction_state");
    }

    std::string to_csv_row() const {
        std::ostringstream os;
        os << impl << ','
           << run_meta_data.profile << ','
           << trace_path << ','
           << run_meta_data.N << ','
           << run_meta_data.seed << ','
           << elapsed_ms() << ','
           << total_ops() << ','
           << inserts << ','
           << erases << ','
           << hash_table_stats;  // append the hash table CSV stats
        return os.str();
    }
};