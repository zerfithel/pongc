<div align="center">

<h1>[ pongc ]<br><a href="https://github.com/zerfithel/pongc/releases">Releases</a></h1>
PongC is a multithreaded implementation of a retro-game „Pong” in pure C, it uses GPU-accelerated rendering using OpenGL and implements a P2P two-player mode.

</div>

# Features

![demo](demo.gif)

- Multithreaded (network & game-logic thread)
- P2P architecture
- GPU-Acceleration in OpenGL
- Low-level optimizations (thanks to being written in C)
- Hackable shaders and game settings

# Install from package

You can find pre-compiled binaries for both Windows and Linux in [releases page](https://github.com/zerfithel/pongc/releases).

# Install from source

Dependencies:
- git *
- cmake *
- make *
- C23+ compiler *
- SDL2
- OpenGL 3.3+
- libenet

*_make depends_

The build is CMake-based. After installing the dependencies run the following commands.

```bash
git clone https://github.com/zerfithel/pongc && cd pongc
cmake -B build -DCMAKE_BUILD_TYPE=Release # Build for production
cmake -B build -DCMAKE_BUILD_TYPE=Debug   # Build for debugging
cd build && make -j$(nproc)
./pongc
```
> If you are compiling on Windows, you might wanna add `-G MinGW Makefiles` to cmake command or you could use another building system.

In order to install PongC run following command if you are using Linux or Unix/Unix-like system:
```bash
chmod +x scripts/install.sh && scripts/install.sh
```

If you are using Windows, please run instead:
```bash
.\scripts\install.bat
```

# Project layout

```
├── CMakeLists.txt  CMake build file
├── demo.gif        Game demo GIF
├── LICENSE.txt     Project LICENSE
├── README.md       This file
├── scripts         Associated to project scripts
├── tests/          Tests for project
├── docs/           Documentation
└── src/            PongC source code
```

# Contributors ❤️

![Contributors](https://contrib.rocks/image?repo=zerfithel/pongc)

# License

PongC is available under [MIT License](LICENSE.txt)
