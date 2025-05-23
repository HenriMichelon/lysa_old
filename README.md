# Lysa

A 3D game engine.

## Features

- GPU-driven rendering
- Node-based (heavily inspired by [Godot](https://godotengine.org/))
- Vulkan 1.3 & DirectX 12 support
- Selectable forward and deferred renderers
- Physics support
- Bundled PBR shader & post-processing shaders
- Written in and for modern C++
- [Slang shader language](https://shader-slang.org/) support for CMake
- Math library with Slang-like syntax

## Third party dependencies used

- [Vireo RHI](https://github.com/HenriMichelon/vireo_rhi) : A 3D Rendering Hardware Interface
- [HLSL++](https://github.com/redorav/hlslpp/) : A math library using HLSL syntax with multiplatform SIMD support by Emilio López
- [JoltPhysics](https://github.com/jrouwe/JoltPhysics) : A multicore-friendly rigid body physics and collision detection library by Jorrit Rouwe
