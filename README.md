# Ignis

![](https://github.com/Nielsbishere/ignis/workflows/C%2FC++%20CI/badge.svg)

## Description

Ignis is a very minimal abstraction layer (no dependencies needed) between modern graphics APIs; current list of planned APIs:

- OpenGL 4.6 (In development)
- Vulkan 1.0 (In development)

Current list of planned platforms:

- Windows (In development)
- Linux (In development)
- Android (Planned; Vulkan only)

The main idea is to allow fast library prototyping with optimal performance and low load times.

Since this is a no-dependency library, it won't provide resource management and it is highly recommended to add a resource manager that loads textures, shaders (SPIR-V) and other data such as shader reflection. This also means you have to manage your mips yourself; something similar to DDS with mips should be used.

## Setup on windows

- Download CMake 3.13 or higher
- Download Visual Studio or a CMake generator of your choosing
- Run `mkdir builds & cd builds & cmake .. -G "<Your generator here>`
- Open the solution or run `cmake --build .`

## Setup on linux

- Download cmake (`sudo apt install cmake`)
- Download mesa and their opengl packages (`sudo apt install mesa-common-dev && sudo apt install libgl1-mesa-dev && sudo apt install libglu1-mesa-dev`)
- Run `mkdir builds && cd builds && cmake ..`
- Run `make` or `cmake --build .`

## Builds

```bat
git clone --recursive https://github.com/Nielsbishere/ignis
mkdir builds & cd builds
cmake ../ -G "Visual Studio 16 2019" -A x64
cmake --build . -j 8
cd ../
```

## Guides

These are guides on how Ignis works:

- [ViewportInterface](docs/ViewportInterface.md)
