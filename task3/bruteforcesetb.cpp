#include "bruteforce.h"
#include <iostream>
#include <thread>
#include <cmath>
#include <mutex>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <openssl/sha.h>

using namespace std;

// Thread safe output
mutex cout_mutex;

PrefixFilter::PrefixFilter() : valid_prefixes{false} {}

// Add prefix to valid set
void PrefixFilter::add_prefix(uint32_t prefix) {
    if (prefix < valid_prefixes.size()) {
        valid_prefixes[prefix] = true;
    }
}

// Validates prefix
bool PrefixFilter::is_valid_prefix(uint64_t number) const noexcept {
    while (number > 0) {
        // Iteratively check prefixes and remove last digit
        if (number < valid_prefixes.size() && valid_prefixes[number]) {
            return true;
        }
        number /= 10; 
    }
    return false;
}

// Validation check according to luhn algorithm
bool LuhnGenerator::is_valid(uint64_t number) noexcept {
    int sum = 0;
    bool alternate = false;
    while (number > 0) {
        // Fetch last digit
        int digit = number % 10;
        if (alternate) {
            // Double digit subtract 9 if results > 9
            digit *= 2;
            if (digit > 9) digit -= 9;
        }
        sum += digit;
        alternate = !alternate;
        // Remove last digit
        number /= 10;
    }
    // Check if sum modulo 10 = 0
    return sum % 10 == 0;
}

// BruteForce class constructor
BruteForce::BruteForce(const vector<string>& hashes) 
    : found(hashes.size(), false), 
      start_time(hashes.size()), 
      end_time(hashes.size()),
      found_count(0),
      all_found_flag(false),
      global_counter(0),
      total_combinations(0),
      last_progress_report(std::chrono::steady_clock::now()) {

    program_start_time = chrono::high_resolution_clock::now();
    cout << "Starting initialization..." << endl;

    // String to raw bytes conversion
    for (const auto& hash : hashes) {
        array<unsigned char, HASH_DIGEST_LENGTH> raw_hash;
        for (size_t i = 0; i < HASH_DIGEST_LENGTH; ++i) {
            raw_hash[i] = stoul(hash.substr(i*2, 2), nullptr, 16);
        }
        target_hashes.push_back(raw_hash);
    }

    precompute_prefixes();
    initialiseBINRanges();
}

// Precompute hash prefix's and store them
void BruteForce::precompute_prefixes() {
    for (size_t i = 0; i < target_hashes.size(); ++i) {
        // Combine 4 bytes into uint32_t with bitwise operations
        uint32_t prefix = (uint32_t(target_hashes[i][0]) << 24) | (uint32_t(target_hashes[i][1]) << 16) |
                          (uint32_t(target_hashes[i][2]) << 8)  | uint32_t(target_hashes[i][3]);
        // Reverse byte order
        prefix = __builtin_bswap32(prefix);
        // Extract upper 16 bits with right shift
        prefix_map[prefix >> 16].push_back(i);
    }
}

// Initialise BIN (Bank Identification Number) ranges
void BruteForce::initialiseBINRanges() {
    auto add_range = [this](uint32_t start, uint32_t end, uint8_t length) {
        for (uint32_t i = start; i <= end; ++i) {
            bin_ranges.push_back({i, length, 0});
            prefix_filter.add_prefix(i);
        }
    };

    // Add BIN ranges here
    add_range(34, 34, 15);  // AMEX
    add_range(37, 37, 15);  // AMEX
    add_range(62, 62, 16);  // China UnionPay
    add_range(300, 305, 14);// Diners Club
    add_range(36, 36, 14);  // Diners Club
    add_range(54, 55, 16);  // Diners Club
    add_range(6011, 6011, 16);  // Discover
    add_range(622126, 622925, 16);  // Discover
    add_range(644, 649, 16);    // Discover
    add_range(65, 65, 16);  // Discover
    add_range(636, 636, 16);// InterPayment
    add_range(637, 639, 16);// InstaPayment
    add_range(3528, 3589, 16);  // JCP
    // Maestro
    for (uint32_t i : {50, 56, 57, 58, 6}) {
        for (uint8_t len = 12; len <= 16; ++len) {
            bin_ranges.push_back({i, len, 0});
            prefix_filter.add_prefix(i);
        }
    }
    add_range(2221, 2720, 16);  // Mastercard
    add_range(51, 55, 16);  // Mastercard
    add_range(4, 4, 13);    // Visa
    add_range(4, 4, 16);    // Visa

    // Calculate total combinations possible
    total_combinations = 0;
    for (auto& range : bin_ranges) {
        // Calculate number of digits for each prefix
        int prefix_length = (range.prefix == 0) ? 1 : static_cast<int>(log10(range.prefix)) + 1;
        // Calculation for range = 10^(total_length - prefix_length) 
        range.count = static_cast<uint64_t>(pow(10, range.length - prefix_length));
        total_combinations += range.count;
    }
}
// Brute force a segment of CC numbers
void BruteForce::bf_seg(uint64_t seg_start, uint64_t seg_end, const BINRange& range) {
    // Calculate multiplier for prefix and variable combination
    uint64_t prefix_multiplier = pow(10, range.length - to_string(range.prefix).length());
    
    SHA_CTX sha_ctx;
    unsigned char hash_bytes[HASH_DIGEST_LENGTH];

    for (uint64_t i = seg_start; i < seg_end && !all_found_flag; ++i) {
        // Combine prefix with variable to form full number
        uint64_t number = range.prefix * prefix_multiplier + i;

        if (LuhnGenerator::is_valid(number)) {
            // Calculate SHA1 hash of the CC number
            SHA1_Init(&sha_ctx);
            SHA1_Update(&sha_ctx, &number, sizeof(number));
            SHA1_Final(hash_bytes, &sha_ctx);

            // Extract first 4 bytes of hash and reverse byte order
            uint32_t hash_prefix = __builtin_bswap32(*(uint32_t*)hash_bytes);
            
            // Use the upper 16 bits as key in prefix map
            // Check to see if hash matches any of the target hashes without completing a full comparison
            auto it = prefix_map.find(hash_prefix >> 16);
            if (it != prefix_map.end()) {
                // If matching prefix is found compare full hash
                for (size_t h : it->second) {
                    if (memcmp(hash_bytes, target_hashes[h].data(), HASH_DIGEST_LENGTH) == 0) {
                        // Full hash match found
                        lock_guard<mutex> lock(cout_mutex);
                        if (!found[h]) {
                            found[h] = true;
                            end_time[h] = chrono::high_resolution_clock::now();
                            cout << "Credit Card Number found: " << number << endl;
                            if (++found_count == target_hashes.size()) {
                                all_found_flag = true;
                                return;
                            }
                        }
                    }
                }
            }
        }

        global_counter.fetch_add(1, std::memory_order_relaxed);

        // Print progress every 5 minutes
        auto now = std::chrono::steady_clock::now();
        if (now - last_progress_report >= std::chrono::minutes(5)) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            uint64_t current_count = global_counter.load(std::memory_order_relaxed);
            double progress_percentage = (static_cast<double>(current_count) / total_combinations) * 100.0;
            
            std::cout << "Progress: " << current_count << " / " << total_combinations 
                      << " (" << std::fixed << std::setprecision(6) << progress_percentage << "%)" << std::endl;
            
            last_progress_report = now;
        }
    }
}

// Run function for bruteforce attack
void BruteForce::run() {
    cout << "Starting brute force search..." << endl;
    auto search_start_time = chrono::high_resolution_clock::now();

    for (size_t i = 0; i < target_hashes.size(); ++i) {
        start_time[i] = chrono::high_resolution_clock::now();
    }

    unsigned int num_threads = thread::hardware_concurrency();
    vector<thread> threads;

    for (const auto& range : bin_ranges) {
        // Divide work among the threads
        uint64_t seg_size = range.count / num_threads;
        for (unsigned int i = 0; i < num_threads; ++i) {
            uint64_t seg_start = i * seg_size;
            // Ensure the last thread processes remaining numbers
            uint64_t seg_end = (i == num_threads - 1) ? range.count : (i + 1) * seg_size;
            threads.emplace_back(&BruteForce::bf_seg, this, seg_start, seg_end, range);
        }

        for (auto& thread : threads) {
            thread.join();
        }

        threads.clear();

        if (all_found_flag) break;
    }

    auto search_end_time = chrono::high_resolution_clock::now();
    auto search_duration = chrono::duration_cast<chrono::milliseconds>(search_end_time - search_start_time);
    cout << "Brute force search completed in " << search_duration.count() << " ms" << endl;

    // Print results
    for (size_t i = 0; i < target_hashes.size(); ++i) {
        if (found[i]) {
            auto duration = chrono::duration_cast<chrono::milliseconds>(end_time[i] - start_time[i]);
            cout << "Credit Card hash " << i + 1 << " cracked in " << duration.count() << " milliseconds." << endl;
        } else {
            cout << "Credit Card hash " << i + 1 << " not cracked." << endl;
        }
    }

    auto total_duration = chrono::duration_cast<chrono::milliseconds>(search_end_time - program_start_time);
    cout << "Total program execution time: " << total_duration.count() << " ms" << endl;
}

// Driver code
int main() {
    vector<string> hashes = {
        "e263d987c8ab258772dfef42722208683ac6f9fd", // 
        "eb7000c09d332fcaf961aa8d25dd96efc84e34c6", // 
        "b02132081808b493c61e86626ee6c2e29326a662"  // 0000000000000000
    };

    BruteForce bruteForce(hashes);
    bruteForce.run();

    return 0;
}