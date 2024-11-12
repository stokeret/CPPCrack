#ifndef BRUTEFORCE_H
#define BRUTEFORCE_H

#include <vector>
#include <string>
#include <array>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <openssl/sha.h>

// Effectively the same as the SHA_DIGEST_LENGTH var previously used but cpp was unhappy so I re-declared it as HASH_DIGEST_LENGTH
constexpr size_t HASH_DIGEST_LENGTH = 20;

// Prefix filter class def
class PrefixFilter {
private:
    std::array<bool, 1000000> valid_prefixes; // Bitset to store the prefixes

public:
    PrefixFilter();
    void add_prefix(uint32_t prefix);
    [[nodiscard]] bool is_valid_prefix(uint64_t number) const noexcept;
};

// Class def for luhn validation
class LuhnGenerator {
public:
    static bool is_valid(uint64_t number) noexcept;
};

// BINRange struct def
struct BINRange {
    uint32_t prefix;
    uint8_t length;
    uint64_t count;
};

// BruteForce class def
class BruteForce {
private:
    std::vector<std::array<unsigned char, HASH_DIGEST_LENGTH>> target_hashes;
    std::vector<bool> found;
    std::vector<std::chrono::high_resolution_clock::time_point> start_time, end_time;
    std::atomic<int> found_count;
    std::atomic<bool> all_found_flag;
    std::atomic<uint64_t> global_counter;
    uint64_t total_combinations;
    std::chrono::high_resolution_clock::time_point program_start_time;
    std::chrono::steady_clock::time_point last_progress_report;

    PrefixFilter prefix_filter;
    std::vector<BINRange> bin_ranges;
    std::unordered_map<uint16_t, std::vector<size_t>> prefix_map;

    void precompute_prefixes();
    void initialiseBINRanges();
    void bf_seg(uint64_t seg_start, uint64_t seg_end, const BINRange& range);

public:
    BruteForce(const std::vector<std::string>& hashes);
    void run();
};

#endif // BRUTEFORCE_H