# c-codebase
My Codebase for C programming.
Based a little on https://www.youtube.com/c/Mr4thProgramming but a lot more is added here

Mainly made to use as few libraries as possible and for my own exploration and use.

## What does this codebase have

- A Base Layer:
  - Basic Memory Allocators
  - String Utilities
  - Thread Context
  - Math
  - Logging
  - Generic Data structure defining macros
- An OS Layer: (Only win32 implemented):
  - Basic File handling
  - Dynamic Library Loading
  - Windowing
  - Input
  - Timing
  - Threading
- A Core Graphics Layer:
  - Multiple Backends (OpenGL 3.3, OpenGL 4.6 and D3D11)
  - OS Specific Device/Context Creation
  - OpenGL Function Loading
  - Thin wrapper over backend specific calls (Buffers, Pipelines and Shaders are generic to all backends)
- Several Optional Layers
  - Simple 2D Renderer
  - User Interface (Somewhat complete)
  - Simple 2D Physics (Only collision detection and resolution done. No dynamics yet)

## How do I use this codebase?

It has a simple build script batch file.
Just cd into the directory and run `build.bat`!

You need clang installed as I have not tested this with other compilers.
Windows Only because I can't test things out with Linux since I don't have a Linux machine.
