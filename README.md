# pongc - Classic pong game but... multithreaded and multiplayer!

**pongc** is a implementation of classic pong game in C with SDL2 and ENET to play multiplayer through network.

- pongc in action:
![Gameplay](demo.gif)

---

# Technology stack used

- **SDL2**    - Drawing, handling input, window & render
- **ENet**    - Network code (server & client)
- **threads** - Multithreaded code, main thread (drawing, input) & network thread (calculations, network communications), mutexes and atomic variables from `<stdatomic.h>`.

---

# Project layout

```
pongc/          zerfithel/pongc repository
├── include/       pongc header files
├── build/         directory for building
├── tutorial/      tutorial on how to write your own pong!
├── LICENSE.txt    pongc license
├── Makefile       building for Linux
├── README.md      this file
└── src/           pongc source (header files are in include/)
    ├── client.c      client source code
    ├── game.c        main thread loop
    ├── ball.c        ball physics
    ├── main.c        start (init structures, libs and run threads loops)
    ├── signals.c     signal handlers and senders
    ├── server.c      server source code
    └── utils.c       string/math helpers/utils
```

# Building from source

1. Clone this repository:
```bash
git clone https://github.com/zerfithel/pongc
cd pongc
```

2. Build project:
```bash
cd build
cmake ..
make
```

3. Run game:
```bash
./pongc --join <ip:port>
./pongc --host <port>
```

---

# How it works?

I will write a full `.pdf` file documentation that will explain each file, function and how the code works.

# Plans to do

- Make code better
- Make server more authorative
- Print score on screen
- Synchronise score with server and client
- Write full documentation
- Add 4 player mode (maybe)

---

# License

**pongc** is licensed under MIT license. For more details see [LICENSE.txt](LICENSE.txt).
