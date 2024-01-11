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
[] Material Editor to be able to create materials <- doing
[] Use material system to take control over render styles
[x] Add debug lines
[] Font rendering
[] Profiling and Unit tests

# TODO: unspecified order

- [x] FileIterator(done) and remove last of std::filesystem
- [] Implement Mip-Mapping from Vulkan tutorial
- [] Implement a File watcher
- [] Hookup file-watcher to the reflection system and material system 
- [] Hookup file-watcher to textures and shaders
- [] Make resource registry thread safe, could be done by looking it on write operations and waiting on read

## Important Notes
Create a seperate Command Pool for short lived commands, as well as a transfer Queue only for transfer commands on the GPU.
Iterators for for-each loops on all container types


