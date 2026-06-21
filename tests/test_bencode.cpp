#include "../src/bencode.h"
#include <iostream>
#include <cassert>

int main() {
    using namespace bencode;
    std::string s = "d3:bar4:spam3:fooi42ee"; // {bar: "spam", foo: 42}
    Value v = parse(s);
    assert(v.is_dict());
    auto &d = v.as_dict();
    assert(d.at("bar").is_str());
    assert(d.at("bar").as_str() == "spam");
    assert(d.at("foo").is_int());
    assert(d.at("foo").as_int() == 42);
    std::cout << "bencode test passed\n";
    return 0;
}
