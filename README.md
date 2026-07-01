# ctorrent — BitTorrent client project

This project is a C++ BitTorrent client that is being built in stages. The current codebase already has a working foundation and several core subsystems implemented.

## Done so far

- Project scaffold with CMake, Makefile, Dockerfile, and `.gitignore`
- `bencode` parser for torrent metadata
- `Storage` layer with piece read/write and SHA-1 verification
- Peer wire basics: handshake, messages, and bitfield support
- Tracker client for HTTP announce and tracker response parsing
- Tests for bencode, storage, peer wire, and tracker layers
- CLI entry point that can parse and print a `.torrent` file

## Current build and run

Native build:

```bash
make build
```

Run the current CLI:

```bash
./build/ctorrent path/to/file.torrent
```

Run tests:

```bash
make test
```

Docker build and run:

```bash
make docker-build
make docker-run
```

## Next implementation steps

- Peer management: connections, choking/unchoke, rate limits
- Piece selection, pipelining, and endgame handling
- DHT for magnet lookup and peer discovery
- Persistence for resume files, config, and logging
- Better CLI and optional lightweight web UI
- Additional integration tests and optional protocol features

