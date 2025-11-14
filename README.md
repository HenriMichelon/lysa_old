# Lysa

A 3D Engine.

[User documentation](https://henrimichelon.github.io/Lysa/)

## Some Features

- GPU-driven rendering.
- [Vulkan 1.3](https://www.vulkan.org/) & DirectX 12 support.
- Physics engine with [Jolt Physics 5](https://github.com/jrouwe/JoltPhysics) and [NVIDIA PhysX 5](https://github.com/NVIDIA-Omniverse/PhysX) support.
- Node-based scene tree with a classical OOP approach.
- Selectable forward and deferred renderers.
- Support for PBR, Bloom, SSAO, HDR tone-mapping (Reinhard and ACES), SMAA, FXAA.
- Written in and for modern C++.
- Math library with Slang-like syntax.
- [Slang shader language](https://shader-slang.org/) support for CMake.

## Third party dependencies used

- [Vireo RHI](https://github.com/HenriMichelon/vireo_rhi) : A 3D Rendering Hardware Interface
- [HLSL++](https://github.com/redorav/hlslpp/) : A math library using HLSL syntax with multiplatform SIMD support by Emilio López
- [xxHash](https://github.com/Cyan4973/xxHash) : Extremely fast non-cryptographic hash algorithm by Yann Collet
- [JoltPhysics](https://github.com/jrouwe/JoltPhysics) : A multicore-friendly rigid body physics and collision detection library by Jorrit Rouwe
- [PhysX](https://github.com/NVIDIA-Omniverse/PhysX) : A physics library by NVIDIA
- [json](https://github.com/nlohmann/json) : JSON for Modern C++ by Niels Lohmann
