<<<<<<< HEAD
\# rawrender



A CPU software renderer written from scratch in C. No graphics API. No OpenGL. No DirectX. Every pixel is computed manually and written directly to a Win32 bitmap.



!\[F1 Car Front](screenshots/1.png)

!\[F1 Car Side](screenshots/2.png)

!\[F1 Car Rear](screenshots/3.png)

!\[F1 Car Close](screenshots/4.png)

!\[Cube](screenshots/cube.png)



\## Features



\- Edge function rasterizer with incremental traversal

\- Incremental Z interpolation — no per-pixel barycentric solve, pure additions in the inner loop

\- Depth buffer for correct occlusion

\- Homogeneous clip space triangle clipping

\- SIMD accelerated vertex transformation — AVX2 (8 vertices at a time) and SSE (4 vertices at a time)

\- Diffuse lighting via dot product between transformed surface normal and light direction

\- Win32 windowing — pixels written directly to memory, no intermediate API



\## How to build



Requires MSVC. Open a Developer Command Prompt and run:

```

build.bat

```



\## How to use



The renderer expects mesh data in this format:

```c

typedef struct {

&#x20; vec3\* vertices;  // flat array, every 3 vertices = 1 triangle

&#x20; vec3\* normals;   // one normal per vertex, same indexing

&#x20; int vertex\_count; // must be multiple of 3

&#x20; mat4 model;

} MeshS;

```



Provide your own OBJ loader or any other source that fills this struct. Then call:

```c

draw\_mesh\_simd(engine, mesh, model\_matrix, buffer, x\_mvp, y\_mvp, z\_mvp, w\_mvp, frequency);

```



\## What I learned



\- How GPU rasterization pipelines work at the hardware level

\- SIMD intrinsics (AVX2/SSE) for parallel floating point math

\- Homogeneous clipping and perspective correct interpolation

\- Edge functions and incremental attribute interpolation

\- Win32 windowing and raw bitmap rendering

=======
# rawRender
>>>>>>> 254a638c9994b08ee8b9ed51aa0bfd483ced7770
