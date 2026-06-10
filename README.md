# exchange-demo

A from-scratch limit order book and matching engine in modern C++, structured as a small exchange simulation. Built as a learning project to explore the systems-level concerns behind low-latency trading infrastructure: data-oriented design, cache-friendly data structures, and a clean component boundary between the book, the matching logic, and the surrounding pipeline.

## Status

Work in progress. The order book core is in place; the matching engine and market-data path are being built out. The goal is to work on functionality and correctness and then performance

## Building

Requires CMake (3.28+) and a C++26-capable compiler (GCC 16+).

```bash
cmake -S . -B build
cmake --build build
```

Run the simulation:

```bash
./build/apps/exchange_sim/exchange_sim
```

