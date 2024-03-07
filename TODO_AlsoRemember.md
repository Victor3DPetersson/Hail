# TODO:
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
[] Memory allocators & memory pool
[] Red-black tree and hashmap
[] Long string class
[] Make asserts and error Handling
[x] Update the input handler and create an input to action map
[] Threaded loading, make it safe to load from game thread
[] Animation system for 2D animations
[] Profiling and a profiler window
[] Imgui Dockable
[] Use material system to take control over render styles and add blending
[] Implement Mip-Mapping
[] Implement a File watcher
[] Make resource registry thread safe, could be done by looking it on write operations and waiting on read
[] Font rendering
[] 2D camera for the 2D rendering

## Important Notes
Create a seperate Command Pool for short lived commands, as well as a transfer Queue only for transfer commands on the GPU.
Iterators for for-each loops on all container types


