Base code for Computer Graphics Assignments
===========================================

Author: Tamy Boubekeur (tamy.boubekeur@telecom-paristech.fr)

### Building

This is a standard CMake project. Building it consits in running:

```
cd <path-to-BaseGL-directory>
mkdir build
cd build
cmake ..
cd ..
cmake --build build
```

The resuling BaseGL executable is automatically copied to the root BaseGL directory, so that resources (shaders, meshes) can be loaded easily. By default, the program is compile in Debug mode. For a high performance Release binary, just us:

```
cmake --build build --config Release
```

### Running

To run the program
```
cd <path-to-BaseGL-directory>
./BaseGL [file.off]
```
Note that a collection of example meshes are provided in the Resources/Models directory. 

When starting to edit the source code, rerun 

```
cmake --build build 
```

to recompile. The resulting binary to use is always the one at located in the BaseGL directory, you can safely ignore whatever is generated in the build directory. 
