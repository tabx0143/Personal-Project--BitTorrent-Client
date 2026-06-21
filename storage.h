#pragma once

#include <string>
#include <vector>
#include <cstdint>

class Storage {
public:
    Storage(const std::string& path, uint64_t total_size, uint32_t piece_length);
    ~Storage();

    // Write an entire piece (exactly piece_length bytes, except last piece)
    void write_piece(uint32_t index, const std::vector<unsigned char>& data);

    // Read a piece into buffer (resizes to piece size)
    std::vector<unsigned char> read_piece(uint32_t index);

    // Verify piece SHA1 (20-byte binary expected)
    bool verify_piece(uint32_t index, const std::vector<unsigned char>& expected_sha1);

    uint32_t piece_length() const { return piece_length_; }
    uint64_t total_size() const { return total_size_; }

private:
    std::string path_;
    uint64_t total_size_;
    uint32_t piece_length_;
};
