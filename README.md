# Hail
 Custom game engine - framework for c++ game development

# TODO: unspecified order for Hail

	- Improve growing array
- Memory allocators & memory pool (check out the Arena memory allocator strategy)
- Red-black tree and hashmap
	- Long string class
- Text render commands
	- Font rendering
	- Make asserts and error Handling
	- Update the input handler and create an input to action map
- Threaded loading, make it safe to load from game thread
- Animation system for 2D animations
- Profiling and a profiler window
- Imgui Dockable
	- Use material system to take control over render styles and add blending
	- Shader owned by material Instances
- Configurable rendering, so start of a render graph
- Compute shaders, dependent on Configurable rendering
- Implement Mip-Mapping
- Implement a File watcher
- Make resource registry thread safe, could be done by locking it on write operations and waiting on read
	- 2D camera for the 2D rendering
- Resource registration Init Check if current path does not match the meta resource project path, and make a function to clean that up.
- Implement batch rendering of sprites
- Spline objects
	- Implement VMA on the Vulkan backend (buffers are implemented)
- VMA implement texture support
	- Angelscript, implement array
- Angelscript, replace std::string with our own string class
	- Angelscript, add debugging support in VS code
	- Angelscript, implement the input handler and debug commands to the scripts
- Angelscript, implement the language server protocol for SyntaxHighlighting in VS-Code.
- Angelscript, improve hot reloading and make hotreloading when changing dependency files
- Fix broken reloading of GPU resources and then remake hot reloading 
	- Context upload once function 
- Context, move over rendering and state functions to the context.
- Depth Sorting of sprites
- Explicit setting of Framebuffer bind state through the context object. Make materials transition have a bind state that is not set at Init
- Bindless texturing
- RawInput instead of Windows input
- Input queue to make input more predictable
- Create a render command for sprites and a pipeline to create sprites from AngelScript
- Design and decide over game Objects and level structure

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