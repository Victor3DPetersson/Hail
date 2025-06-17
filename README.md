# Hail
 Custom game engine - framework for c++ game development


# TODO: priority order for project

- [x] Text render commands
- [x] Depth Sorting of sprite and text commands
- [x] Implement batch rendering of sprites and fonts
- [x] Cloud rendering experiment of a point cloud dataset
- [x] First draft Update cloud point data as a fluid for dynamic clouds
- [x] Rewrite the fluid solver not after Sebastians video (Coding Adventure: Simulating Fluids) but this paper: Smoothed Particle Hydrodynamics. Techniques for the Physics Based Simulation of Fluids and Solids
- [x] Compute shader support
- [x] Texture views to have read only, write only and read write access to textures ( might need to add memory barriers for the reading)
- [] Sorting with a compute shader
- [] Update point cloud data with a compute pass and create a sdf texture each frame for all fluids
- [] Angelscript, implement the language server protocol for SyntaxHighlighting in VS-Code.
- [] Create a pipeline to create sprites render commands from AngelScript
- [] Fix broken reloading of GPU resources and then remake hot reloading 


# TODO: unspecified order for Hail Engine

- [] Memory allocators & memory pool (check out the Arena memory allocator strategy)
- [] Red-black tree and hashmap
- [x] Improve growing array
- [x] Long string class
- [x] Text render commands
- [x] Font rendering
- [x] Make asserts and error Handling
- [x] Update the input handler and create an input to action map
- [] Threaded loading, make it safe to load from game thread
- [] Animation system for 2D animations
- [] Profiling and a profiler window
- [] Imgui Dockable
- [x] Use material system to take control over render styles and add blending
- [x] Shader owned by material Instances
- [] Configurable rendering, so start of a render graph
- [x] Compute shader support
- [] Implement Mip-Mapping
- [] Implement a File watcher
- [] Make resource registry thread safe, could be done by locking it on write operations and waiting on read
- [x] 2D camera for the 2D rendering
- [] Resource registration Init Check if current path does not match the meta resource project path, and make a function to clean that up.
- [x] Implement batch rendering of sprites and fonts
- [] Spline objects
- [x] Implement VMA on the Vulkan backend
- [x] VMA implement texture support
- [x] Angelscript, implement array
- [] Angelscript, replace std::string with our own string class
- [x] Angelscript, add debugging support in VS code
- [x] Angelscript, implement the input handler and debug commands to the scripts
- [] Angelscript, implement the language server protocol for SyntaxHighlighting in VS-Code.
- [] Angelscript, improve hot reloading and make hotreloading when changing dependency files
- [] Fix broken reloading of GPU resources and then remake hot reloading 
- [x] Context upload once function 
- [x] Context, move over rendering and state functions to the context.
- [x] Depth Sorting of sprite and text commands
- [x] Explicit setting of Framebuffer bind state through the context object. Make materials transition have a bind state that is not set at Init
- [] Bindless texturing
- [] RawInput instead of Windows input
- [] Input queue to make input more predictable
- [] Create a pipeline to create sprites render commands from AngelScript
- [] Design and decide over game Objects and level structure


## Important Notes
Create a seperate Command Pool for short lived commands, as well as a transfer Queue only for transfer commands on the GPU.
Iterators for for-each loops on all container types


## Known bugs:
Compound glyphs can get the wrong horizontal alignment, repo case, Roboto-Medium ':'


# First todo's when starting out: (keeping them here as it is quite cute to keep)
- Complete Tutorial
- Implement ImGui
- Implement a mesh in a basic way
- Setup basic threaded workflow between renderer and Game to move said mesh
- Imgui interface over threads
- Create several 3D objects and instance render them.
- Sprite component
- Resource Manager (create material system and break out tutorial code)
- Create Serializing so I can make materials be read from files
- Implement an Asset Browser to be able to import textures 
- Resource Registry, so we can at runtime be able to check status of files and what exists
- Material Editor to be able to create materials
- Add debug lines
- FileIterator and remove last of std::filesystem