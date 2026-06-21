#include "storage.h"
#include <openssl/sha.h>
#include <fstream>
#include <stdexcept>
#include <algorithm>

Storage::Storage(const std::string& path, uint64_t total_size, uint32_t piece_length)
    : path_(path), total_size_(total_size), piece_length_(piece_length) {
    // Preallocate file
    std::fstream f(path_, std::ios::in | std::ios::out | std::ios::binary);
    if (!f) {
        // create and set size
        std::ofstream out(path_, std::ios::binary);
        if (!out) throw std::runtime_error("failed to create file");
        if (total_size_ > 0) {
            out.seekp((std::streamoff)total_size_ - 1);
            out.write("\0", 1);
        }
        out.close();
    }
}

Storage::~Storage() {}

void Storage::write_piece(uint32_t index, const std::vector<unsigned char>& data) {
    uint64_t offset = (uint64_t)index * piece_length_;
    if (offset >= total_size_) throw std::runtime_error("piece index out of range");
    uint32_t expected = std::min<uint64_t>(piece_length_, total_size_ - offset);
    if (data.size() != expected) throw std::runtime_error("data size mismatch");
    std::fstream f(path_, std::ios::in | std::ios::out | std::ios::binary);
    if (!f) throw std::runtime_error("failed to open file for write");
    f.seekp((std::streamoff)offset);
    f.write(reinterpret_cast<const char*>(data.data()), data.size());
}

std::vector<unsigned char> Storage::read_piece(uint32_t index) {
    uint64_t offset = (uint64_t)index * piece_length_;
    if (offset >= total_size_) throw std::runtime_error("piece index out of range");
    uint32_t expected = std::min<uint64_t>(piece_length_, total_size_ - offset);
    std::vector<unsigned char> buf(expected);
    std::ifstream f(path_, std::ios::binary);
    if (!f) throw std::runtime_error("failed to open file for read");
    f.seekg((std::streamoff)offset);
    f.read(reinterpret_cast<char*>(buf.data()), expected);
    return buf;
}

bool Storage::verify_piece(uint32_t index, const std::vector<unsigned char>& expected_sha1) {
    auto buf = read_piece(index);
    unsigned char out[SHA_DIGEST_LENGTH];
    SHA1(buf.data(), buf.size(), out);
    return std::equal(out, out + SHA_DIGEST_LENGTH, expected_sha1.begin());
}
