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
I would recommend you to make your own :)
However if you still want to:
It has a simple build script file.
Just cd into the directory and run `build.bat` or `build.sh`!

You need clang OR MSVC installed as I have not tested this with gcc.
AFAIK it should just work with gcc anyways.

Linux is currently WIP

## TODOs
- [x] Linux
    - [x] Windowing with X11
    - [x] Input Handling
    - [x] OpenGL 3.3 backend layer
    - [x] OpenGL 4.6 backend layer
- [ ] Optional Layers
    - [ ] More UI Widgets
- [ ] D3D11
    - [ ] Framebuffer support
- [ ] Cleanup
    - [ ] Switch from X-Lists to Metaprogram tables for OpenGL function loading
    - [ ] Remove Backend specific links (d3d11) by loading functions from dll
- [ ] Major Goals
    - [ ] Custom Build system (so I can combine .bat and .sh build files)


- [ ] Maybes
    - [ ] Implement functions that are insanely tedious (OS_ThreadWaitForAll/Any and others)
