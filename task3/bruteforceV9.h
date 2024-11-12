// Current Version: 9

#ifndef BRUTEFORCE_H
#define BRUTEFORCE_H

#include <string>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <array>
#include <atomic>
#include <openssl/sha.h>

class BruteForce {
public:
    BruteForce(const std::string& charset, int max_len, const std::unordered_set<std::string>& hashes);
    void run();

private:
    std::string charset;
    int max_len;
    std::unordered_set<std::string> hashes;
    std::vector<std::array<unsigned char, SHA_DIGEST_LENGTH>> raw_hashes;  // New member
    std::vector<std::chrono::high_resolution_clock::time_point> start_time;
    std::vector<std::chrono::high_resolution_clock::time_point> end_time;
    std::vector<bool> found;
    int found_count;
    std::atomic<bool> all_found_flag;

    void bf_seg(uint64_t seg_start, uint64_t seg_end, int length, bool reverse);
    bool all_found() const;

    // Helper function to initialise char_to_index
    void init_char_to_index();
};

#endif // BRUTEFORCE_H