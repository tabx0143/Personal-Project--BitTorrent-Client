#include "peer_wire.h"

#include <algorithm>
#include <stdexcept>

namespace peerwire {

namespace {

void write_u32_be(std::vector<unsigned char>& out, std::uint32_t value) {
    out.push_back(static_cast<unsigned char>((value >> 24) & 0xFF));
    out.push_back(static_cast<unsigned char>((value >> 16) & 0xFF));
    out.push_back(static_cast<unsigned char>((value >> 8) & 0xFF));
    out.push_back(static_cast<unsigned char>(value & 0xFF));
}

std::uint32_t read_u32_be(const std::vector<unsigned char>& bytes, std::size_t offset) {
    return (static_cast<std::uint32_t>(bytes[offset]) << 24) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 16) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 8) |
           static_cast<std::uint32_t>(bytes[offset + 3]);
}

} // namespace

Message Message::keep_alive() {
    return Message{};
}

Bitfield::Bitfield(std::size_t piece_count) : pieces_(piece_count, false) {}

void Bitfield::resize(std::size_t piece_count) {
    pieces_.resize(piece_count, false);
}

std::size_t Bitfield::size() const {
    return pieces_.size();
}

bool Bitfield::has(std::size_t piece_index) const {
    if (piece_index >= pieces_.size()) {
        throw std::out_of_range("bitfield index out of range");
    }
    return pieces_[piece_index];
}

void Bitfield::set(std::size_t piece_index, bool value) {
    if (piece_index >= pieces_.size()) {
        throw std::out_of_range("bitfield index out of range");
    }
    pieces_[piece_index] = value;
}

std::vector<unsigned char> Bitfield::serialize() const {
    const std::size_t byte_count = (pieces_.size() + 7) / 8;
    std::vector<unsigned char> payload(byte_count, 0);
    for (std::size_t i = 0; i < pieces_.size(); ++i) {
        if (!pieces_[i]) {
            continue;
        }
        payload[i / 8] |= static_cast<unsigned char>(1u << (7 - (i % 8)));
    }
    return payload;
}

Bitfield Bitfield::parse(const std::vector<unsigned char>& payload, std::size_t piece_count) {
    const std::size_t expected = (piece_count + 7) / 8;
    if (payload.size() != expected) {
        throw std::runtime_error("bitfield payload size mismatch");
    }

    Bitfield bitfield(piece_count);
    for (std::size_t i = 0; i < piece_count; ++i) {
        const unsigned char byte = payload[i / 8];
        bitfield.pieces_[i] = ((byte >> (7 - (i % 8))) & 0x01u) != 0;
    }
    return bitfield;
}

std::vector<unsigned char> encode_handshake(const Handshake& handshake) {
    std::vector<unsigned char> bytes;
    bytes.reserve(kHandshakeSize);
    bytes.push_back(static_cast<unsigned char>(kProtocolLength));
    bytes.insert(bytes.end(), kProtocolString, kProtocolString + kProtocolLength);
    bytes.insert(bytes.end(), handshake.reserved.begin(), handshake.reserved.end());
    bytes.insert(bytes.end(), handshake.info_hash.begin(), handshake.info_hash.end());
    bytes.insert(bytes.end(), handshake.peer_id.begin(), handshake.peer_id.end());
    return bytes;
}

Handshake decode_handshake(const std::vector<unsigned char>& bytes) {
    if (bytes.size() != kHandshakeSize) {
        throw std::runtime_error("invalid handshake size");
    }
    if (bytes[0] != kProtocolLength) {
        throw std::runtime_error("invalid protocol string length");
    }
    if (!std::equal(bytes.begin() + 1, bytes.begin() + 1 + kProtocolLength,
                    reinterpret_cast<const unsigned char*>(kProtocolString))) {
        throw std::runtime_error("invalid protocol string");
    }

    Handshake handshake;
    std::copy(bytes.begin() + 1 + kProtocolLength, bytes.begin() + 1 + kProtocolLength + 8,
              handshake.reserved.begin());
    std::copy(bytes.begin() + 1 + kProtocolLength + 8,
              bytes.begin() + 1 + kProtocolLength + 8 + 20, handshake.info_hash.begin());
    std::copy(bytes.begin() + 1 + kProtocolLength + 8 + 20, bytes.end(), handshake.peer_id.begin());
    return handshake;
}

Message make_message(std::uint8_t id, const std::vector<unsigned char>& payload) {
    Message message;
    message.id = id;
    message.payload = payload;
    return message;
}

std::vector<unsigned char> encode_message(const Message& message) {
    std::vector<unsigned char> bytes;
    if (!message.id.has_value()) {
        bytes.resize(4, 0);
        return bytes;
    }
    const std::uint32_t length = static_cast<std::uint32_t>(1 + message.payload.size());
    write_u32_be(bytes, length);
    bytes.push_back(*message.id);
    bytes.insert(bytes.end(), message.payload.begin(), message.payload.end());
    return bytes;
}

std::optional<Message> decode_message(const std::vector<unsigned char>& bytes) {
    if (bytes.size() < 4) {
        throw std::runtime_error("message too short");
    }

    const std::uint32_t length = read_u32_be(bytes, 0);
    if (length == 0) {
        return Message::keep_alive();
    }

    if (bytes.size() != 4 + length) {
        throw std::runtime_error("message length mismatch");
    }

    Message message;
    message.id = bytes[4];
    message.payload.assign(bytes.begin() + 5, bytes.end());
    return message;
}

} // namespace peerwire
