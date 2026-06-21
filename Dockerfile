FROM ubuntu:22.04
RUN apt-get update && apt-get install -y build-essential cmake ca-certificates && rm -rf /var/lib/apt/lists/*
WORKDIR /app
COPY . /app
RUN cmake -S . -B build && cmake --build build -- -j
CMD ["/app/build/ctorrent"]
