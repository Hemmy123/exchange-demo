# exchange-demo

A from-scratch limit order book and matching engine in modern C++, structured as a small exchange simulation. Built as a learning project to explore the systems-level concerns behind low-latency trading infrastructure: data-oriented design, cache-friendly data structures, and a clean component boundary between the book, the matching logic, and the surrounding pipeline.

## Status

Work in progress. The order book core is in place; the matching engine and market-data path are being built out. The goal is to work on functionality and correctness and then performance

## Notes on AI Usage:

The project was created to assist in learning, as such AI was not used to generate the code. However it was used in other areas lists below:

- CMake configuration. I've had experiences using other build systems in the past, but surprisingly CMAKE is a new experience for me. Since I wanted to focus on the C++ and low level machine knowledge, AI assistance was used to help create the CMAKE scripts

- Unit tests. Currently most units tests are hand written, but some longer complex are AI generated or assisted.

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

