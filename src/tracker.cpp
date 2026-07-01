#include "tracker.h"

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace tracker {

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

namespace {

std::string hex_byte(unsigned char c) {
    std::ostringstream oss;
    oss << '%' << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<int>(c);
    return oss.str();
}

std::string encode_binary(const unsigned char* data, std::size_t size) {
    std::string out;
    out.reserve(size * 3);
    for (std::size_t i = 0; i < size; ++i) {
        const unsigned char c = data[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            out.push_back(static_cast<char>(c));
        } else {
            out += hex_byte(c);
        }
    }
    return out;
}

std::string event_to_string(Event event) {
    switch (event) {
    case Event::None: return "";
    case Event::Started: return "started";
    case Event::Stopped: return "stopped";
    case Event::Completed: return "completed";
    }
    return "";
}

std::string get_str(const bencode::Dict& d, const std::string& key) {
    auto it = d.find(key);
    if (it == d.end() || !it->second.is_str()) {
        throw std::runtime_error("tracker response missing string key: " + key);
    }
    return it->second.as_str();
}

std::uint32_t get_uint(const bencode::Dict& d, const std::string& key) {
    auto it = d.find(key);
    if (it == d.end() || !it->second.is_int()) {
        throw std::runtime_error("tracker response missing integer key: " + key);
    }
    return static_cast<std::uint32_t>(it->second.as_int());
}

std::vector<PeerEndpoint> parse_compact_peers(const std::string& peers) {
    if (peers.size() % 6 != 0) {
        throw std::runtime_error("invalid compact peer list length");
    }
    std::vector<PeerEndpoint> out;
    out.reserve(peers.size() / 6);
    for (std::size_t i = 0; i < peers.size(); i += 6) {
        const unsigned char a = static_cast<unsigned char>(peers[i]);
        const unsigned char b = static_cast<unsigned char>(peers[i + 1]);
        const unsigned char c = static_cast<unsigned char>(peers[i + 2]);
        const unsigned char d = static_cast<unsigned char>(peers[i + 3]);
        const std::uint16_t port = (static_cast<std::uint16_t>(static_cast<unsigned char>(peers[i + 4])) << 8) |
                                   static_cast<std::uint16_t>(static_cast<unsigned char>(peers[i + 5]));
        std::ostringstream ip;
        ip << static_cast<int>(a) << '.' << static_cast<int>(b) << '.' << static_cast<int>(c) << '.' << static_cast<int>(d);
        out.push_back(PeerEndpoint{ip.str(), port});
    }
    return out;
}

struct ParsedUrl {
    std::string host;
    std::string port;
    std::string target;
};

ParsedUrl parse_http_url(const std::string& url) {
    const std::string prefix = "http://";
    if (url.rfind(prefix, 0) != 0) {
        throw std::runtime_error("only http:// trackers are supported");
    }
    ParsedUrl parsed;
    const std::string rest = url.substr(prefix.size());
    const auto slash = rest.find('/');
    const std::string authority = slash == std::string::npos ? rest : rest.substr(0, slash);
    parsed.target = slash == std::string::npos ? "/" : rest.substr(slash);

    const auto colon = authority.find(':');
    if (colon == std::string::npos) {
        parsed.host = authority;
        parsed.port = "80";
    } else {
        parsed.host = authority.substr(0, colon);
        parsed.port = authority.substr(colon + 1);
    }
    return parsed;
}

} // namespace

std::string build_announce_url(const AnnounceRequest& request) {
    std::ostringstream url;
    url << request.announce_url;
    url << (request.announce_url.find('?') == std::string::npos ? '?' : '&');
    url << "info_hash=" << encode_binary(request.info_hash.data(), request.info_hash.size());
    url << "&peer_id=" << encode_binary(request.peer_id.data(), request.peer_id.size());
    url << "&port=" << request.port;
    url << "&uploaded=" << request.uploaded;
    url << "&downloaded=" << request.downloaded;
    url << "&left=" << request.left;
    url << "&compact=" << (request.compact ? 1 : 0);
    const auto event = event_to_string(request.event);
    if (!event.empty()) {
        url << "&event=" << event;
    }
    return url.str();
}

AnnounceResponse parse_announce_response(const std::string& body) {
    const auto value = bencode::parse(body);
    if (!value.is_dict()) {
        throw std::runtime_error("tracker response is not a dictionary");
    }

    const auto& dict = value.as_dict();
    AnnounceResponse response;

    auto interval_it = dict.find("interval");
    if (interval_it != dict.end() && interval_it->second.is_int()) {
        response.interval = static_cast<std::uint32_t>(interval_it->second.as_int());
    }

    auto complete_it = dict.find("complete");
    if (complete_it != dict.end() && complete_it->second.is_int()) {
        response.complete = static_cast<std::uint32_t>(complete_it->second.as_int());
    }

    auto incomplete_it = dict.find("incomplete");
    if (incomplete_it != dict.end() && incomplete_it->second.is_int()) {
        response.incomplete = static_cast<std::uint32_t>(incomplete_it->second.as_int());
    }

    auto peers_it = dict.find("peers");
    if (peers_it == dict.end()) {
        return response;
    }

    if (peers_it->second.is_str()) {
        response.peers = parse_compact_peers(peers_it->second.as_str());
        return response;
    }

    if (peers_it->second.is_list()) {
        for (const auto& item : peers_it->second.as_list()) {
            if (!item.is_dict()) {
                continue;
            }
            const auto& peer_dict = item.as_dict();
            PeerEndpoint endpoint;
            endpoint.ip = get_str(peer_dict, "ip");
            endpoint.port = static_cast<std::uint16_t>(get_uint(peer_dict, "port"));
            response.peers.push_back(std::move(endpoint));
        }
    }

    return response;
}

AnnounceResponse announce_http(const AnnounceRequest& request) {
    const auto parsed_url = parse_http_url(request.announce_url);
    const auto full_url = build_announce_url(request);

    asio::io_context ioc;
    asio::ip::tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);
    const auto results = resolver.resolve(parsed_url.host, parsed_url.port);
    stream.connect(results);

    http::request<http::string_body> req{http::verb::get, parsed_url.target, 11};
    req.set(http::field::host, parsed_url.host);
    req.set(http::field::user_agent, "ctorrent/0.1");
    req.target(full_url.substr(full_url.find(parsed_url.target)));

    http::write(stream, req);

    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res);

    beast::error_code ec;
    stream.socket().shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    if (ec == asio::error::not_connected) {
        ec = {};
    }
    if (ec) {
        throw beast::system_error{ec};
    }

    if (res.result() != http::status::ok) {
        throw std::runtime_error("tracker request failed with HTTP status " + std::to_string(res.result_int()));
    }

    return parse_announce_response(res.body());
}

} // namespace tracker
