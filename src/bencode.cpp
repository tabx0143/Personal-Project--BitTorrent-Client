#include "bencode.h"
#include <stdexcept>
#include <cctype>

namespace bencode {

struct Parser {
    const std::string& s;
    size_t i = 0;
    Parser(const std::string& data) : s(data) {}

    char peek() const { return i < s.size() ? s[i] : '\0'; }
    char get() { return i < s.size() ? s[i++] : '\0'; }

    Value parse_value() {
        char c = peek();
        if (c == 'i') return parse_int();
        if (std::isdigit((unsigned char)c)) return parse_str();
        if (c == 'l') return parse_list();
        if (c == 'd') return parse_dict();
        throw std::runtime_error("invalid bencode data");
    }

    Value parse_int() {
        // i<digits>e
        if (get() != 'i') throw std::runtime_error("expected 'i'");
        bool neg = false;
        if (peek() == '-') { neg = true; get(); }
        long long val = 0;
        if (!std::isdigit((unsigned char)peek())) throw std::runtime_error("invalid integer");
        while (std::isdigit((unsigned char)peek())) {
            val = val * 10 + (get() - '0');
        }
        if (get() != 'e') throw std::runtime_error("unterminated integer");
        if (neg) val = -val;
        Value V; V.v = val; return V;
    }

    Value parse_str() {
        // <len>:<data>
        size_t len = 0;
        if (!std::isdigit((unsigned char)peek())) throw std::runtime_error("invalid string length");
        while (std::isdigit((unsigned char)peek())) { len = len*10 + (get() - '0'); }
        if (get() != ':') throw std::runtime_error("expected ':' after string length");
        if (i + len > s.size()) throw std::runtime_error("string extends past end");
        std::string out = s.substr(i, len);
        i += len;
        Value V; V.v = std::move(out); return V;
    }

    Value parse_list() {
        if (get() != 'l') throw std::runtime_error("expected 'l'");
        List lst;
        while (peek() != 'e') {
            lst.push_back(parse_value());
        }
        get(); // consume 'e'
        Value V; V.v = std::move(lst); return V;
    }

    Value parse_dict() {
        if (get() != 'd') throw std::runtime_error("expected 'd'");
        Dict d;
        while (peek() != 'e') {
            Value key = parse_str();
            if (!key.is_str()) throw std::runtime_error("dict key not string");
            Value val = parse_value();
            d.emplace(key.as_str(), std::move(val));
        }
        get();
        Value V; V.v = std::move(d); return V;
    }
};

// Value helpers
bool Value::is_int() const { return std::holds_alternative<long long>(v); }
bool Value::is_str() const { return std::holds_alternative<std::string>(v); }
bool Value::is_list() const { return std::holds_alternative<List>(v); }
bool Value::is_dict() const { return std::holds_alternative<Dict>(v); }

long long Value::as_int() const { return std::get<long long>(v); }
const std::string& Value::as_str() const { return std::get<std::string>(v); }
const List& Value::as_list() const { return std::get<List>(v); }
const Dict& Value::as_dict() const { return std::get<Dict>(v); }

Value parse(const std::string& data) {
    Parser p(data);
    Value v = p.parse_value();
    return v;
}

} // namespace bencode
