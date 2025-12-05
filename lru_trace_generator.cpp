#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <list>

void generateTrace(size_t N, unsigned seed, const std::vector<std::string> &all_words) {
    if (all_words.size() < 4 * N) {
        std::cerr << "Error: Not enough word pairs for N=" << N << std::endl;
        return;
    }

    std::vector<std::string> accesses;

    // 1N words
    for (size_t i = 0; i < N; ++i) {
        accesses.push_back(all_words[i]);
    }

    // 1N words 5 times each
    for (size_t i = N; i < 2 * N; ++i) {
        for (int j = 0; j < 5; ++j) {
            accesses.push_back(all_words[i]);
        }
    }

    // 2N words accessed 3 times each
    for (size_t i = 2 * N; i < 4 * N; ++i) {
        for (int j = 0; j < 3; ++j) {
            accesses.push_back(all_words[i]);
        }
    }

    // Shuffle with seed
    std::mt19937 rng(seed);
    std::shuffle(accesses.begin(), accesses.end(), rng);

    std::list<std::string> LRU_list;
    std::unordered_map<std::string, std::list<std::string>::iterator> resident_map;


    // Create output file
    std::string filename = "traceFiles/lru_profile_N_" + std::to_string(N) + "_S_" + std::to_string(seed) + ".trace";
    std::ofstream outfile(filename);
    if (!outfile) {
        std::cerr << "Error: Cannot create " << filename << std::endl;
        std::cerr << "Make sure 'traceFiles' directory exists." << std::endl;
        return;
    }

    outfile << "lru_profile " << N << " " << seed << "\n";

//    For each word w:
//    - If w is already in resident_map, it is a hit; do:
//    - Use the address of the node to move w to the most-recent position in LRU_list.
//    - Emit _I w_.
//    - If w is not resident (miss) and resident_map has fewer than N values:
//    – Insert w in resident_map.
//    – Place w at the most-recent end of the LRU_list.
//    – Emit I w.
//      - If w is not in resident_map (miss) and the size is N. Call the word at the least-recently used end of the LRU_list y.
//    – Delete the entry in resident_map that contains y.
//    – Delete the word at the least-recently used end of LRU_list.
//    – Insert w in the resident_map.
//    – Add w to the most-recently used end of LRU_list.
//    – Emit E y.
//    – Emit I w.
//            end-for

    for (const auto& w : accesses) {
        auto it = resident_map.find(w);

        if (it != resident_map.end()) {
            LRU_list.splice(LRU_list.end(), LRU_list, it->second);
            outfile << "I " << w << "\n";

        } else if (resident_map.size() < N) {
            LRU_list.push_back(w);
            resident_map[w] = --LRU_list.end();
            outfile << "I " << w << "\n";

        } else {
            std::string y = LRU_list.front();
            resident_map.erase(y);
            LRU_list.pop_front();

            outfile << "E " << y << "\n";

            LRU_list.push_back(w);
            resident_map[w] = --LRU_list.end();

            outfile << "I " << w << "\n";
        }
    }

    outfile.close();
    std::cout << "Generated " << filename << std::endl;
}

int main() {
    const std::string words_file = "20980712_uniq_words.txt";
    const unsigned seed = 23;

    // Read word pairs from file
    std::ifstream infile(words_file);
    if (!infile) {
        std::cerr << "Error: Cannot open " << words_file << std::endl;
        return 1;
    }

    std::vector<std::string> all_words;

    std::string word1, word2;
    while (infile >> word1 >> word2) {
        all_words.push_back(word1 + " " + word2);
    }
    infile.close();

    std::cout << "Read " << all_words.size() << " word pairs from " << words_file << std::endl;

    // Generate traces for N from 2^10 to 2^20
    size_t N = 1024;
    for (int power = 10; power <= 20; power++) {
        generateTrace(N, seed, all_words);
        N *= 2;
    }

    std::cout << "\nAll traces generated." << std::endl;

    return 0;
}