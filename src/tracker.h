#pragma once

#include "bencode.h"
#include "peer_wire.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace tracker {

enum class Event {
    None,
    Started,
    Stopped,
    Completed,
};

struct AnnounceRequest {
    std::string announce_url;
    peerwire::InfoHash info_hash{};
    peerwire::PeerId peer_id{};
    std::uint16_t port = 6881;
    std::uint64_t uploaded = 0;
    std::uint64_t downloaded = 0;
    std::uint64_t left = 0;
    bool compact = true;
    Event event = Event::None;
};

struct PeerEndpoint {
    std::string ip;
    std::uint16_t port = 0;
};

struct AnnounceResponse {
    std::optional<std::uint32_t> interval;
    std::optional<std::uint32_t> complete;
    std::optional<std::uint32_t> incomplete;
    std::vector<PeerEndpoint> peers;
};

std::string build_announce_url(const AnnounceRequest& request);
AnnounceResponse parse_announce_response(const std::string& body);

// Synchronous HTTP announce for http:// trackers.
AnnounceResponse announce_http(const AnnounceRequest& request);

} // namespace tracker
