#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <filesystem>

#include "HashTableDictionary.hpp"
#include "Operations.hpp"
#include "RunMetaData.hpp"
#include "RunResults.hpp"

bool load_trace_strict_header(const std::string &path, RunMetaData &runMeta, std::vector<Operation> &out_operations) {
    std::string profile = "";
    std::size_t N = 0;
    int seed = 0;
    out_operations.clear();

    std::ifstream in(path);
    if (!in.is_open()) {
        std::cerr << "Error: Cannot open trace file: " << path << std::endl;
        return false;
    }

    std::string header;
    if (!std::getline(in, header))
        return false;

    const auto first = header.find_first_not_of(" \t\r\n");
    if (first == std::string::npos || header[first] == '#')
        return false;

    std::istringstream hdr(header);
    if (!(hdr >> profile >> N >> seed)) {
        std::cerr << "Error: Invalid header format in " << path << std::endl;
        return false;
    }

    runMeta.profile = profile;
    runMeta.N = N;
    runMeta.seed = seed;

    std::string line;
    while (std::getline(in, line)) {
        const auto opCodeIdx = line.find_first_not_of(" \t\r\n");
        if (opCodeIdx == std::string::npos || line[opCodeIdx] == '#')
            continue;

        std::istringstream iss(line.substr(opCodeIdx));
        std::string tok;
        if (!(iss >> tok))
            continue;

        std::string w1, w2;
        if (tok == "I") {
            if (!(iss >> w1 >> w2)) {
                std::cerr << "Error: Invalid Insert operation in " << path << std::endl;
                return false;
            }
            out_operations.emplace_back(OpCode::Insert, w1 + " " + w2);
        } else if (tok == "E") {
            if (!(iss >> w1 >> w2)) {
                std::cerr << "Error: Invalid Erase operation in " << path << std::endl;
                return false;
            }
            out_operations.emplace_back(OpCode::Erase, w1 + " " + w2);
        } else {
            std::cerr << "Error: Unknown operation '" << tok << "' in " << path << std::endl;
            return false;
        }
    }

    return true;
}

std::size_t tableSizeForN(std::size_t N) {
    static const std::vector<std::pair<std::size_t, std::size_t>> N_and_primes = {
            { 1024,    1279    },
            { 2048,    2551    },
            { 4096,    5101    },
            { 8192,   10273    },
            { 16384,  20479    },
            { 32768,  40849    },
            { 65536,  81931    },
            { 131072, 163861   },
            { 262144, 327739   },
            { 524288, 655243   },
            { 1048576, 1310809 }
    };

    for (const auto& [n, prime] : N_and_primes) {
        if (n == N)
            return prime;
    }

    std::cerr << "Error: Unable to find table size for N=" << N << std::endl;
    std::exit(1);
}

RunResult run_trace_ops(HashTableDictionary &hashTable,
                        RunResult &runResult,
                        const std::vector<Operation> &ops) {

    // Count operations
    for (const auto &op : ops) {
        switch (op.tag) {
            case OpCode::Insert:
                ++runResult.inserts;
                break;
            case OpCode::Erase:
                ++runResult.erases;
                break;
        }
    }

    // One untimed warm-up run
    hashTable.clear();
    for (const auto &op : ops) {
        switch (op.tag) {
            case OpCode::Insert:
                hashTable.insert(op.key);
                break;
            case OpCode::Erase:
                (void) hashTable.remove(op.key);
                break;
        }
    }

    // Seven timed runs
    using clock = std::chrono::steady_clock;
    const int numTrials = 7;
    std::vector<std::int64_t> trials_ns;
    trials_ns.reserve(numTrials);

    for (int i = 0; i < numTrials; ++i) {
        hashTable.clear();
        auto t0 = clock::now();

        for (const auto &op : ops) {
            switch (op.tag) {
                case OpCode::Insert:
                    hashTable.insert(op.key);
                    break;
                case OpCode::Erase:
                    (void) hashTable.remove(op.key);
                    break;
            }
        }

        auto t1 = clock::now();
        trials_ns.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
    }

    // Calculate median
    const size_t mid = trials_ns.size() / 2;
    std::nth_element(trials_ns.begin(), trials_ns.begin() + mid, trials_ns.end());
    runResult.elapsed_ns = trials_ns[mid];

    runResult.hash_table_stats = hashTable.csvStats();

    return runResult;
}

void find_trace_files(const std::string &profile_prefix,
                      std::vector<std::string> &out_files) {
    namespace fs = std::filesystem;
    out_files.clear();

    fs::path trace_dir("traceFiles");
    const std::string suffix = ".trace";

    std::error_code ec;
    if (!fs::exists(trace_dir, ec)) {
        std::cerr << "Error: 'traceFiles' directory does not exist" << std::endl;
        return;
    }

    if (!fs::is_directory(trace_dir, ec)) {
        std::cerr << "Error: 'traceFiles' exists but is not a directory" << std::endl;
        return;
    }

    for (const auto &entry : fs::directory_iterator(trace_dir, ec)) {
        if (ec) continue;
        if (!entry.is_regular_file(ec)) continue;

        const std::string name = entry.path().filename().string();
        const bool has_prefix = (name.rfind(profile_prefix, 0) == 0);
        const bool has_suffix = name.size() >= suffix.size() &&
                                name.compare(name.size() - suffix.size(),
                                             suffix.size(), suffix) == 0;

        if (has_prefix && has_suffix) {
            out_files.push_back(entry.path().string());
        }
    }

    std::sort(out_files.begin(), out_files.end());
}

int main(int argc, char* argv[]) {
    std::vector<std::string> traceFiles;
    find_trace_files("lru_profile", traceFiles);

    if (traceFiles.empty()) {
        std::cerr << "Error: No trace files found." << std::endl;
        return 1;
    }

    std::cerr << "Found " << traceFiles.size() << " trace file(s)" << std::endl;

    std::vector<RunResult> runResults;

    for (const auto &traceFile : traceFiles) {
        const auto pos = traceFile.find_last_of("/\\");
        auto traceFileBaseName = (pos == std::string::npos) ? traceFile : traceFile.substr(pos + 1);

        std::vector<Operation> operations;
        RunMetaData run_meta_data;

        if (!load_trace_strict_header(traceFile, run_meta_data, operations)) {
            std::cerr << "Error: Failed to load " << traceFile << std::endl;
            continue;
        }

        std::cerr << "Processing: " << traceFileBaseName
                  << " (N=" << run_meta_data.N << ", ops=" << operations.size() << ")" << std::endl;

        const std::size_t tableSize = tableSizeForN(run_meta_data.N);

        // Run with DOUBLE probing
        {
            RunResult result_double(run_meta_data);
            result_double.impl = "hash_map_double";
            result_double.trace_path = traceFileBaseName;

            HashTableDictionary hashTable(tableSize,
                                          HashTableDictionary::DOUBLE,
                                          true);

            run_trace_ops(hashTable, result_double, operations);
            runResults.push_back(result_double);
        }

        // Run with SINGLE probing
        {
            RunResult result_single(run_meta_data);
            result_single.impl = "hash_map_single";
            result_single.trace_path = traceFileBaseName;

            HashTableDictionary hashTable(tableSize,
                                          HashTableDictionary::SINGLE,
                                          true);

            run_trace_ops(hashTable, result_single, operations);
            runResults.push_back(result_single);
        }
    }

    // Write CSV output
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    std::stringstream filename_ss;
    filename_ss << "csvs/results_" << time_t_now << ".csv";
    std::string csv_filename = filename_ss.str();

    std::ofstream csv_file(csv_filename);
    if (!csv_file.is_open()) {
        std::cerr << "Error: Could not create CSV file. Make sure 'csvs' directory exists." << std::endl;
        return 1;
    }

    csv_file << RunResult::csv_header() << std::endl;
    for (const auto &run : runResults) {
        csv_file << run.to_csv_row() << std::endl;
    }
    csv_file.close();

    std::cerr << "\nCompleted! " << runResults.size() << " runs -> " << csv_filename << std::endl;

    return 0;
}