# NeVK

## Project Dependencies

- Vulkan  - https://vulkan.lunarg.com/   *implicit*
- glfw    - https://www.glfw.org/     *dll*
- slang      - https://github.com/shader-slang/slang *dll*
- cxxopts   - https://github.com/jarro2783/cxxopts  *header*
- json - https://github.com/nlohmann/json *header*
- imgui   - https://github.com/ocornut/imgui *header+source*
- glm      - https://github.com/g-truc/glm *submodule*
- stb       - https://github.com/nothings/stb *submodule*
- tinygltf    - https://github.com/syoyo/tinygltf *submodule*
- tol - https://github.com/tinyobjloader/tinyobjloader *submodule*
- doctest      - https://github.com/onqtam/doctest *submodule*

## OSX Guide

#### Installation
Follow setup guide https://vulkan-tutorial.com/Development_environment

Clone the project.
   
    git clone https://github.com/ikryukov/NeVK --recursive

#### Launch
Use vscode with preset env variable
1. export VULKAN_SDK=~/vulkansdk/macOS
2. launch code 
    
## Synopsis 

      nevk [MODEL PATH] [OPTION...] positional parameters
      
        -m, --mesh arg     mesh path (default: misc/Cube/Cube.gltf)
            --width arg    window width (default: 800)
            --height arg   window height (default: 600)
        -h, --help         Print usage

## Example

    ./nevk misc/Cube/Cube.gltf

## USD
    
    Install plugin:
        cmake --install . --component HdNeVK
