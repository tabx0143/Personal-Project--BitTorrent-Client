#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace peerwire {

constexpr std::size_t kProtocolLength = 19;
constexpr char kProtocolString[] = "BitTorrent protocol";
constexpr std::size_t kHandshakeSize = 49 + kProtocolLength;

using InfoHash = std::array<unsigned char, 20>;
using PeerId = std::array<unsigned char, 20>;

struct Handshake {
    InfoHash info_hash{};
    PeerId peer_id{};
    std::array<unsigned char, 8> reserved{};
};

struct Message {
    std::optional<std::uint8_t> id;
    std::vector<unsigned char> payload;

    static Message keep_alive();
};

class Bitfield {
public:
    explicit Bitfield(std::size_t piece_count = 0);

    void resize(std::size_t piece_count);
    std::size_t size() const;
    bool has(std::size_t piece_index) const;
    void set(std::size_t piece_index, bool value = true);

    std::vector<unsigned char> serialize() const;
    static Bitfield parse(const std::vector<unsigned char>& payload, std::size_t piece_count);

private:
    std::vector<bool> pieces_;
};

std::vector<unsigned char> encode_handshake(const Handshake& handshake);
Handshake decode_handshake(const std::vector<unsigned char>& bytes);

std::vector<unsigned char> encode_message(const Message& message);
std::optional<Message> decode_message(const std::vector<unsigned char>& bytes);

Message make_message(std::uint8_t id, const std::vector<unsigned char>& payload = {});

} // namespace peerwire
