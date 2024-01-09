# TODO:
1. [x] Complete Tutorial
2. [x] Implement ImGui
3. [x] Implement a mesh in a basic way
4. [x] Setup basic threaded workflow between renderer and Game to move said mesh
5. [x] Imgui interface over threads
6. [x] Create several 3D objects and instance render them.
7. [x] Sprite component
8. [x] Resource Manager (create material system and break out tutorial code)
9. [x] Create Serializing so I can make materials be read from files
10. [] Material Editor and Asset Browser to browse and save files <- doing
11. [] Use material system to take control over render styles <- doing
12. [x] Add debug lines
13. [] Font rendering
14. [] Profiling and Unit tests
s

# TODO: unspecified order

- [x] FileIterator(done) and remove last of std::filesystem
- [] Implement Mip-Mapping from Vulkan tutorial
- [] Implement a File watcher <- doing
- [] Hookup file-watcher to the reflection system and material system 
- [] Hookup file-watcher to textures and shaders

## Important Notes
Create a seperate Command Pool for short lived commands, as well as a transfer Queue only for transfer commands on the GPU.
Growing array Iterators for for-each loops


