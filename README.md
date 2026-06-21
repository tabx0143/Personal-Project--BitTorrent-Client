# ctorrent — minimal BitTorrent client scaffold

This repo contains a minimal scaffold for a BitTorrent client in C++.

What’s included:
- `bencode` parser library (supports ints, strings, lists, dicts)
- `ctorrent` CLI that parses a `.torrent` file and prints its structure
- `Makefile` with `build`, `test`, and Docker targets
- `Dockerfile` to build and run the project in a container

Build & run (native):

```bash
make build
./build/ctorrent path/to/file.torrent
```

Run tests:

```bash
make test
```

Docker:

```bash
make docker-build
make docker-run
```

Next steps I will implement (unless you request changes): storage layer, piece verification, peer wire protocol, tracker client, DHT, and UI.
