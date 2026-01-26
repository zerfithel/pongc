# pongc - Classic pong game but... multithreaded and multiplayer!

**pongc** is a implementation of classic pong game in C with SDL2 and ENET to play multiplayer through network.

- pongc in action:
![Gameplay](demo.gif)

---

# Technology stack used

- **SDL2** - Creating window, handling input
- **OpenGL** - Drawing on window, shaders
- **ENet** - Network communication with UDP
- **Threads** - Multithreaded code (`<threads.h>`)

---

# Project layout

```
zerfithel/pongc
├── CMakeLists.txt - CMake file
├── demo.gif       - Demo video
├── LICENSE.txt    - License (MIT)
├── README.md      - This file
├── scripts/       - Shell scripts
├── build/         - Directory for building
├── include/       - Header files
└── src            - Source code
    ├── main.c     -- Start of the program
    ├── game.c     -- Main thread
    ├── ball.c     -- Ball physics
    ├── client.c   -- Network thread (client)
    ├── server.c   -- Network thread (server)
    ├── signals.c  -- Signal handlers and senders
    ├── utils.c    -- Math/string utils
    └── shaders.c  -- Utils for shaders
```

# Building from source

## Pre-requisites:

- C Compiler (C23 or higher recommended)
- SDL2
- libenet
- OpenGL (GLEW)
- git
- cmake, make (optional)
- shell (optional)

The project uses `<threads.h>` for threads which isn't fully supported on Windows, if you are Windows user then use the [tinycthread](https://github.com/tinycthread/tinycthread). More info in Windows building guide 

---

## Building on Windows
1. Clone this repository:
```powershell
git clone https://github.com/zerfithel/pongc
cd pongc
```

2. Clone `tinycthread` and copy it into the project:
```powershell
git clone https://github.com/tinycthread/tinycthread
cp tinycthread/source/tinycthread.c src/tinycthread.c
cp tinycthread/source/tinycthread.h include/tinycthread.h
```

3. Add `tinycthread.c` to `CMakeLists.txt` in `SOURCES`:
```cmake
set (SOURCES, 
    ...
    src/tinycthread.c
)
```

4. Replace all `<threads.h>` with `"tinycthread.h"` in all `.c` files inside `src/` directory

5. Build:
```powershell
mkdir build; cd build
cmake ..
make
```

6. Install:
```powershell
cd ..
scripts\install.bat
```

7. Run pongc:
```powershell
pongc --host <port>
pongc --join <ip:port>
```

---

## Building on Linux

1. Clone this repository:
```bash
git clone https://github.com/zerfithel/pongc
cd pongc
```

2. Build:
```bash
mkdir build; cd build
cmake ..
make
```

3. Install:
```bash
cd ..
scripts/install.sh
```

4. Run pongc:
```bash
pongc --host <port>
pongc --join <ip:port>
```

> **Info:** If you don't have port forwarding then you will have to use hamachi to play with your friends. This applies to both Windows and Linux.

---

# Uninstalling

If you wish to uninstall the game use the `uninstall` script in the project. After installing, it should be in:

- Linux:
```
~/.local/share/scripts/uninstall.sh
```

- Windows:
```
%APPDATA%/pongc/scripts/uninstall.sh
```

---

# Plans to do in next update

- Make server more authorative (client only sends input to server and not authorative messages)
- Print score on screen
- Synchronise score with server and client
- Add fancy effects (like motion blur after the ball, animations and sounds, soundtrack etc.)

---

# License

**pongc** is licensed under MIT license. For more details see [LICENSE.txt](LICENSE.txt).
