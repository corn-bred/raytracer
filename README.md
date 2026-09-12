A standard hybrid renderer with progressive pathtracing.

For school, but I just took it as an excuse to make an awesome project. Following "Raytracing in One Weekend", but interpreted (in my own code) into C++ and OpenGL.

## Supports:
- Triangle intersection
- Bounding Volume Hierarchies
- Custom model loading
- Emissors
- Dielectrics
- Metals
- Lambertians
- UV albedo & roughness textures
- Smooth shading

Pass 1: Makes G-buffers to pass to Passes 2 & 3
Pass 2: Rasterizer pass (Direct light)
Pass 3: Pathtracer pass (Indirect light)
Pass 4: Combines both passes and gamma correction adjustment

Basically finished except some visual bugs

Demo:

[![Video Title](https://img.youtube.com/vi/MupE6MsuAt4/0.jpg)](https://www.youtube.com/watch?v=MupE6MsuAt4)

Triangles in scene: 67919

Specs:
CPU: AMD Ryzen 5 4500 6-Core Processor
GPU: NVIDIA GTX 1070
32 GB RAM

## How to build
Requirements: CMake version 4.0 or higher & Ninja
### Non-statically-linked build command:
Run `.\buildexe.bat`
This will build and create an executable in `bin`.
Run `.\bin\Raytracing.exe` in the origin folder (`raytracer\`)
### Statically-linked build command:
Add `-static` as a flag for `buildexe.bat`, so `.\buildexe.bat -static`