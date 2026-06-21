#pragma once

#include <string>
#include <variant>
#include <vector>
#include <map>

namespace bencode {

struct Value;
using List = std::vector<Value>;
using Dict = std::map<std::string, Value>;

struct Value {
    using val_t = std::variant<long long, std::string, List, Dict>;
    val_t v;

    bool is_int() const;
    bool is_str() const;
    bool is_list() const;
    bool is_dict() const;

    long long as_int() const;
    const std::string& as_str() const;
    const List& as_list() const;
    const Dict& as_dict() const;
};

// Parse bencoded data from memory. Throws std::runtime_error on error.
Value parse(const std::string& data);

} // namespace bencode
