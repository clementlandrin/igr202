WARNING : OpenGl 4.5 is needed to run the project.

# Building

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

# Running

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

# Using

## Moving the 3D model

To move the 3D model, use the mouse:

-left click + move the mouse to rotate
-middle click + move the mouse to zoom-in or zoom-out
-right click + move the mouse to move the model over X and Y direction

## Physically-Based Rendering

PBR was implemented using GGX microfacet model. It uses material albedo parameters that can be imported from a texture and a number of lights that can be changed.

### Changing the number of lights

To use more or less lights, use keyboard's up and down arrows. The lights are designed according to the 3-points lighting method.

### Enabling texturing

To enable or disable teXturing, press the X key. By default, it textures the model with brick appearance. To add new textures, create a new subfolder in Resources/Material and add the texture files. The texture can be changed using the MATERIAL_NAME variable in Main.cpp. To do so, replace

```
static const std::string MATERIAL_NAME ("Brick/");
```

by

```
static const std::string MATERIAL_NAME ("Name/");
```

where Name is the name of any subfolder in Resources/Material.

### Enabling normal-mapping

To enable or disable Normal-mapping, press the N key. The texture used for normal-mapping is the one used in the Resources/Material/MATERIAL_NAME folder. As the UV coordinates of the textures does not fit the 3D model, normal discontinuities can be perceived along the plane Z = 0.

## Toon-shading

While PBR aims to render images as photorealistic as possible, Toon-shading is a non-photorealistic rendering based on expressived styles.

### Enabling Toon-shading

To enable and disable Toon-shading, press the T key.

### Default Toon-shading

The default Toon-shading is a commonly used 1D texturing. The colors:

-black for the edges
-white for highly specular areas (i.e. light is mainly reflected in the direction of the camera)
-green for the rest

Default shading mode can be recovered using the 1 numeric key.

### X-Toon shading

X-Toon shading is a method described in the paper X-Toon: An Extended Toon Shader written by Pascal Barla. Instead of providing a 1D texture to perform and expressive rendering, a 2D texture can be used. This way, the toon-shading is extended by a criteria. List of commonly used criteria:

-distance to the camera along camera axis (press the 2 numeric key)
-distance to the camera along focal axis (press the 3 numeric key)
-orientation of the surface with respect to the camera (press the 4 numeric key)

The resulting effect is that color changes are more sudden in the farthest regions than in the nearest ones. This way, less attention is paid to the silhouette. This effect only depends on the used texture.

## Modifying the model structure

In several applications, 3D models are not rendered in totality but modified before. For example, there is no need to render all the details of a far tree if only three pixels are affected by this model.

### Reload the 3D model

Playing with the sections below will distord the model with no doubt.
To load the proper initial 3D model, press the F5 key.

### Filtering

A Laplacian filtering can be performed. The idea is to move vertices along their Laplacian to filter details. To perform a Laplacian filtering, press the 0 1 or 2 keys from the numeric keyboard. The lower is the pressed key symbol, the better is the filtering. But the higher is the pressed key symbol, the faster the filtering converges.

-0 moves vertices from 0.1 of the Laplacian vector
-1 moves vertices from the half of the Laplacian vector
-2 moves vertices from the entire Laplacian vector

### Simplification

The 3D model can be simplified using clustering.

To Simplificate the model with a predefined resolution, press the S key.
To simplificate the model using an Adaptaive resolution implemented with an octree, press the A key.
