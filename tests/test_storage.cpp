#include "../src/storage.h"
#include <openssl/sha.h>
#include <iostream>
#include <vector>
#include <cassert>
#include <unistd.h>

std::vector<unsigned char> sha1_of(const std::vector<unsigned char>& data) {
    std::vector<unsigned char> out(SHA_DIGEST_LENGTH);
    SHA1(data.data(), data.size(), out.data());
    return out;
}

int main() {
    const uint32_t piece_len = 1024;
    const uint64_t total = piece_len * 4; // 4 pieces
    const std::string path = "test_data.bin";
    // remove if exists
    unlink(path.c_str());

    Storage s(path, total, piece_len);

    // create sample piece
    std::vector<unsigned char> piece(piece_len);
    for (uint32_t i = 0; i < piece_len; ++i) piece[i] = (unsigned char)(i & 0xFF);
    auto sha = sha1_of(piece);

    s.write_piece(0, piece);
    auto readback = s.read_piece(0);
    assert(readback == piece);
    bool ok = s.verify_piece(0, sha);
    assert(ok);
    std::cout << "storage test passed\n";
    return 0;
}
