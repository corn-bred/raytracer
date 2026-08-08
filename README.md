# Raytracer
A standard progressive raytracer.

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
- Next Event Estimation (NEE)
- Russian roulette

Example:
![render 1](render1.png)
Triangles in scene: 67919

Non-statically-linked build command:
`g++ src/main.cpp src/glad.c src/shaders.cpp -o bin/main.exe -I include -L lib -lglfw3dll -lassimp -lz`
Statically-linked build command:
`g++ src/main.cpp src/glad.c src/shaders.cpp -o bin/main.exe -L "lib" -I "include" -static -static-libgcc -static-libstdc++ -lglfw3 -lopengl32 -lgdi32 -lassimp -lz`