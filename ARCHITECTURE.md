# PongC - Architecture

If you wish to become project contributor, please take a look at [CONTRIBUTING.md](CONTRIBUTING.md) before opening your PR. Remember to run `make format` before every commit.

## High-level overview

PongC is multithreaded Pong implementation in pure C with GPU-Accelerated rendering with OpenGL and P2P two-player mode. Rendering and input are handled with SDL and OpenGL (GLEW), networking is done with ENet. Program uses shared in-memory state protected by a mutex for communication between program threads.

## Key files

- `src/main.c` -- Bootstrap
    - Parse CLI arguments.
    - Initialize libraries
    - Initialize and run threads
    - Cleanup at exit

- `src/game.c` -- Game thread
    - Handles window evemts
    - Handles user input
    - Calculates physics
    - Renders game

- `src/ball.c` -- Physics (helper)
    - Helpers for game thread to calculate ball physics

- `src/signals.c` -- Network signals (helper)
    - Network packets handlers
    - Network packets senders

- `src/cmdline.c` -- CLI argument parsing (helper)
    - Helpers for parsing CLI arguments

Helper functions SHOULD NOT be thread-safe, before calling helper function on shared data a mutex should be used before and after calling function.

## Protocol (PCPROTO)

PCProto is a text-based peer-to-peer network protocol designed specificially for PongC, made on top of UDP for speed and simplicity.

| Packet | Format | Use |
|--------|--------|-----|
| POS    | `pos;<float:y>` | Player position on Y axis update |
| BALL   | `ball;<float:x>,<float:y>,<float:dx>,<float:dy>,<float:speed>` | Ball position, moving direction and speed update | 
| SERVER_FULL | `server_full` | Informing joining client that another peer is alraedy connected to host |

## Threads

- Thread #1 (`main`)
    - Handling window events
    - Handling player input
    - Handling game physics
    - Rendering

- Thread #2 (`network`)
    - Receiving data from peer
    - Sending data to peer

Both threads are having a pointer to `SharedData` structure that is protected with mutex. SharedData structure is defined in `src/shared.h`.

## Build & Run

1. Build

- Build for release:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DENABLE_TEST=ON
cmake --build build --parallel $(nproc)
```

- Build for debugging:
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DENABLE_TEST=ON
cmake --build build --parallel $(nproc)
```

2. Run:

- Linux / Unix:
```bash
build/pongc
```

- Windows:
```powershell
.\build\pongc.exe
```
