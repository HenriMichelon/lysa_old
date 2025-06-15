# Lysa

A 3D Game Engine.

## (Futures) Features

- GPU-driven rendering
- [Vulkan 1.3](https://www.vulkan.org/) & DirectX 12 support
- Physics engine with [Jolt Physics 5](https://github.com/jrouwe/JoltPhysics) and [NVIDIA PhysX 5](https://github.com/NVIDIA-Omniverse/PhysX) support
- Node-based system (heavily inspired by [Godot](https://godotengine.org/))
- Selectable forward and deferred renderers
- Bundled PBR shader & post-processing shaders
- Written in and for modern C++
- [Slang shader language](https://shader-slang.org/) support for CMake
- Math library with Slang-like syntax

## Third party dependencies used

- [Vireo RHI](https://github.com/HenriMichelon/vireo_rhi) : A 3D Rendering Hardware Interface
- [HLSL++](https://github.com/redorav/hlslpp/) : A math library using HLSL syntax with multiplatform SIMD support by Emilio López
- [xxHash](https://github.com/Cyan4973/xxHash) : Extremely fast non-cryptographic hash algorithm by Yann Collet
- [JoltPhysics](https://github.com/jrouwe/JoltPhysics) : A multicore-friendly rigid body physics and collision detection library by Jorrit Rouwe
- [PhysX](https://github.com/NVIDIA-Omniverse/PhysX) : A physics library by NVIDIA
