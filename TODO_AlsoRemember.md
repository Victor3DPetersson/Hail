# First todo's when starting out: (keeping them here as it is quite cute to keep)
[x] Complete Tutorial
[x] Implement ImGui
[x] Implement a mesh in a basic way
[x] Setup basic threaded workflow between renderer and Game to move said mesh
[x] Imgui interface over threads
[x] Create several 3D objects and instance render them.
[x] Sprite component
[x] Resource Manager (create material system and break out tutorial code)
[x] Create Serializing so I can make materials be read from files
[x] Implement an Asset Browser to be able to import textures 
[x] Resource Registry, so we can at runtime be able to check status of files and what exists
[x] Material Editor to be able to create materials
[x] Add debug lines
[x] FileIterator and remove last of std::filesystem


# TODO: unspecified order

[x] Improve growing array
[] Memory allocators & memory pool (check out the Arena memory allocator strategy)
[] Red-black tree and hashmap
[x] Long string class
[] Text render commands
[x] Font rendering
[x] Make asserts and error Handling
[x] Update the input handler and create an input to action map
[] Threaded loading, make it safe to load from game thread
[] Animation system for 2D animations
[] Profiling and a profiler window
[] Imgui Dockable
[x] Use material system to take control over render styles and add blending
[x] Shader owned by material Instances
[] Configurable rendering, so start of a render graph
[] Compute shaders, dependent on Configurable rendering
[] Implement Mip-Mapping
[] Implement a File watcher
[] Make resource registry thread safe, could be done by locking it on write operations and waiting on read
[x] 2D camera for the 2D rendering
[] Resource registration Init Check if current path does not match the meta resource project path, and make a function to clean that up.
[] Implement batch rendering of sprites
[] Spline objects
[] Implement VMA on the Vulkan backend
[x] Angelscript, implement array
[] Angelscript, replace std::string with our own string class
[x] Angelscript, add debugging support in VS code
[x] Angelscript, implement the input handler and debug commands to the scripts
[] Angelscript, implement the language server protocol for SyntaxHighlighting in VS-Code.
[] Fix broken reloading of GPU resources and then remake hot reloading 
[] Context upload once function for buffers to all buffers in Flight
[] Depth Sorting of sprites
[] Explicit setting of Framebuffer bind state through the context object. Make materials have a bind state thjat is not set at Init, and manually set framebuffer state.
[] Bindless texturing
[] RawInput instead of Windows input
[] Input queue to make input more predictable

## Important Notes
Create a seperate Command Pool for short lived commands, as well as a transfer Queue only for transfer commands on the GPU.
Iterators for for-each loops on all container types


## Known bugs:
Compound glyphs can get the wrong horizontal alignment, repo case, Roboto-Medium ':'