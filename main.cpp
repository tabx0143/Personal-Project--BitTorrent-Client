#include "bencode.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

static void print_value(const bencode::Value& v, int indent = 0) {
    string pad(indent, ' ');
    if (v.is_int()) {
        cout << pad << v.as_int() << '\n';
    } else if (v.is_str()) {
        cout << pad << '"' << v.as_str() << '"' << '\n';
    } else if (v.is_list()) {
        cout << pad << "[\n";
        for (auto &e : v.as_list()) print_value(e, indent+2);
        cout << pad << "]\n";
    } else if (v.is_dict()) {
        cout << pad << "{\n";
        for (auto &kv : v.as_dict()) {
            cout << string(indent+2, ' ') << kv.first << ": ";
            if (kv.second.is_str()) cout << '"' << kv.second.as_str() << '"' << '\n';
            else print_value(kv.second, indent+2);
        }
        cout << pad << "}\n";
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Usage: ctorrent <file.torrent>" << endl;
        return 2;
    }
    ifstream f(argv[1], ios::binary);
    if (!f) { cerr << "failed to open file\n"; return 1; }
    std::ostringstream ss; ss << f.rdbuf();
    string data = ss.str();
    try {
        auto v = bencode::parse(data);
        cout << "Parsed top-level bencode:\n";
        print_value(v);
    } catch (const std::exception& e) {
        cerr << "parse error: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
