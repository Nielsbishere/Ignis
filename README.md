## Ignis

Ignis is a very minimal abstraction layer (no dependencies needed) between modern graphics APIs; current list of planned APIs:

- OpenGL 4.6 (In development)
- Vulkan 1.0

Current list of planned platforms:

- Windows (In development)
- Linux
- Android Vulkan

The main idea is to allow fast library prototyping with optimal performance and low load times.

Since this is a no-dependency library, it won't provide resource management and it is highly recommended to add a resource manager that loads textures, shaders (SPIR-V) and other data such as shader reflection. This also means you have to manage your mips yourself; something similar to DDS with mips should be used.

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
