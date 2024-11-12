// Current Version: 9

#include "bruteforceV9.h"
#include <iostream>
#include <thread>
#include <cmath>
#include <mutex>
#include <algorithm>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <array>
#include <openssl/sha.h>

using namespace std;

// Thread-safe console output
mutex cout_mutex;

// Charachter to index lookup table
array<int, 256> char_to_index;

// Vector used for the storage of hashes
vector<array<unsigned char, SHA_DIGEST_LENGTH>> raw_hashes;

// BruteForce class constructor
BruteForce::BruteForce(const string& charset, int max_len, const unordered_set<string>& hashes)
    : charset(charset), max_len(max_len), hashes(hashes),
      start_time(hashes.size()), end_time(hashes.size()), found(hashes.size(), false), 
      found_count(0), all_found_flag(false) {

    // Initialise the char_to_index lookup table, maps ascii values to charset indices
    char_to_index.fill(-1);
    for (int i = 0; i < charset.length(); ++i) {
        char_to_index[static_cast<unsigned char>(charset[i])] = i;
    }    

    // Convert the string hashes to raw bytes for comparison
    for (const auto& hash : hashes) {
        array<unsigned char, SHA_DIGEST_LENGTH> raw_hash;
        for (size_t i = 0; i < SHA_DIGEST_LENGTH; ++i) {
            // Hex string to integer value conversion
            raw_hash[i] = stoi(hash.substr(i*2, 2), nullptr, 16);
        }
        raw_hashes.push_back(raw_hash);
    }
}

// Run function handling the bruteforce attack
void BruteForce::run() {
    int num_threads = thread::hardware_concurrency();
    auto total_start_time = chrono::high_resolution_clock::now();
    for (size_t i = 0; i < hashes.size(); ++i) {
        start_time[i] = chrono::high_resolution_clock::now();
    }

    // Initial pass, checks all combinations for lengths 1 - 3
    for (int length = 1; length <= 3 && !all_found_flag; ++length) {
        cout << "Cracking passwords of length " << length << " (all combinations)" << endl;
        // Calculate total combinations based upon charset.size() variable using pow
        uint64_t total_combinations = static_cast<uint64_t>(pow(charset.size(), length));
        uint64_t combinations_to_check = (length >= 4) ? total_combinations / 2 : total_combinations;
        // Divide work among threads
        uint64_t seg_size = combinations_to_check / num_threads;

        vector<thread> threads;
        for (int i = 0; i < num_threads; i++) {
            uint64_t seg_start = i * seg_size;
            uint64_t seg_end = (i + 1 == num_threads) ? combinations_to_check : (i + 1) * seg_size;
            threads.emplace_back(&BruteForce::bf_seg, this, seg_start, seg_end, length, false);
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }

    // Second pass, checks 50% in reverse order for lengths 4-6
    if (!all_found_flag) {
        for (int length = 4; length <= max_len && !all_found_flag; ++length) {
            cout << "Cracking passwords of length " << length << " (reverse)" << endl;
            uint64_t total_combinations = static_cast<uint64_t>(pow(charset.size(), length));
            // Check half of the combinations
            uint64_t combinations_to_check = total_combinations / 2;
            uint64_t seg_size = combinations_to_check / num_threads;

            vector<thread> threads;
            for (int i = 0; i < num_threads; i++) {
                uint64_t seg_start = i * seg_size;
                uint64_t seg_end = (i + 1 == num_threads) ? combinations_to_check : (i + 1) * seg_size;
                threads.emplace_back(&BruteForce::bf_seg, this, seg_start, seg_end, length, true);
            }

            for (auto& thread : threads) {
                thread.join();
            }
        }
    }

    // Third pass, checks any remaining length 4-6 passwords in regular order 
    if (!all_found_flag) {
        for (int length = 4; length <= max_len && !all_found_flag; ++length) {
            cout << "Cracking remaining passwords of length " << length << " (forward)" << endl;
            uint64_t total_combinations = static_cast<uint64_t>(pow(charset.size(), length));
            uint64_t combinations_to_check = total_combinations / 2;
            uint64_t seg_size = combinations_to_check / num_threads;

            vector<thread> threads;
            for (int i = 0; i < num_threads; i++) {
                uint64_t seg_start = combinations_to_check + i * seg_size;
                uint64_t seg_end = combinations_to_check + ((i + 1 == num_threads) ? combinations_to_check : (i + 1) * seg_size);
                threads.emplace_back(&BruteForce::bf_seg, this, seg_start, seg_end, length, false);
            }

            for (auto& thread : threads) {
                thread.join();
            }
        }
    }

    // Prints results as well as timings to the console
    auto total_end_time = chrono::high_resolution_clock::now();
    auto total_duration = chrono::duration_cast<chrono::milliseconds>(total_end_time - total_start_time);

    for (size_t i = 0; i < hashes.size(); ++i) {
        if (found[i]) {
            auto duration = chrono::duration_cast<chrono::milliseconds>(end_time[i] - start_time[i]);
            cout << "Password hash " << i + 1 << " cracked in " << duration.count() << " milliseconds." << endl;
        } else {
            cout << "Password hash " << i + 1 << " not cracked." << endl;
        }
    }
    cout << "Total time to crack all hashes: " << total_duration.count() << " milliseconds." << endl;
}

// Function used for performing the bruteforce on a segment
void BruteForce::bf_seg(uint64_t seg_start, uint64_t seg_end, int length, bool reverse) {
    int charset_l = charset.length();
    vector<unsigned char> current(length, charset[0]);
    uint64_t num = seg_start;
    
    // Initialise current string
    for (int j = length - 1; j >= 0; j--) {
        current[j] = charset[num % charset_l];
        num /= charset_l;
    }

    // If reverse, start from the end of the charset
    if (reverse) {
        for (unsigned char& c : current) {
            // Invert character in charset
            c = charset[charset_l - 1 - char_to_index[c]];
        }
    }

    SHA_CTX sha_ctx;
    unsigned char hash_bytes[SHA_DIGEST_LENGTH];

    for (uint64_t i = seg_start; i < seg_end && !all_found_flag; i++) {
        // Calculate hash for the current combination
        SHA1_Init(&sha_ctx);
        SHA1_Update(&sha_ctx, current.data(), length);
        SHA1_Final(hash_bytes, &sha_ctx);

        // Compare raw bytes with early exit
        for (size_t h = 0; h < raw_hashes.size(); ++h) {
            bool match = true;
            for (int j = 0; j < SHA_DIGEST_LENGTH; ++j) {
                if (hash_bytes[j] != raw_hashes[h][j]) {
                    match = false;
                    break;
                }
            }
            if (match) {
                lock_guard<mutex> lock(cout_mutex);
                if (!found[h]) {
                    found[h] = true;
                    end_time[h] = chrono::high_resolution_clock::now();
                    cout << "Password found: " << string(current.begin(), current.end()) << endl;
                    cout.flush();
                    found_count++;
                    if (all_found()) {
                        all_found_flag = true;
                        return;
                    }
                }
            }
        }

        // Generate the next combination
        if (reverse) {
            // Decrement combination in reverse lexicographical order e.g. charset = "ABC" then CCC, CCB, CCA, etc.
            for (int j = 0; j < length; j++) {
                int index = char_to_index[current[j]];
                if (index > 0) {
                    current[j] = charset[index - 1];
                    break;
                } else {
                    current[j] = charset[charset_l - 1];
                    if (j == length - 1) {
                        return;
                    }
                }
            }
        } else {
            // Increment combination in forward or regular lexicogrphical order e.g. charset = "ABC" then AAA, AAB, AAC, etc.
            for (int j = length - 1; j >= 0; j--) {
                int index = char_to_index[current[j]];
                if (index < charset_l - 1) {
                    current[j] = charset[index + 1];
                    break;
                } else {
                    current[j] = charset[0];
                    if (j == 0) {
                        return;
                    }
                }
            }
        }
    }
}

// Check to see if all hashes have been found
bool BruteForce::all_found() const {
    return found_count == hashes.size();
}

// Driver code
int main() {
    string charset = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz";
    int max_len = 6;
    unordered_set<string> hashes = {                // Passwords:
        "6abff66b64abeab10a2f08902d0e062db3af4dba", // AbC
        "ecc376ff5b68aeeaa3e1b8a6081b0900ba239cac", // ZzZzZz
        "11f6ad8ec52a2984abaafd7c3b516503785c2072"  // x
    };

    BruteForce bruteForce(charset, max_len, hashes);
    bruteForce.run();

    return 0;
}