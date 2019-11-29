WARNING : OpenGl 4.5 is needed to run the project.

# Summary

* [Using](#-using)
  * [Moving the 3D model](#-moving_the_3d_model)
  * [Physically-Based Rendering](#-physically-based_rendering)
    * [Changing the number of lights](#-changing_the_number_of_lights)
    * [Enabling texturing](#-enabling_texturing)
    * [Enabling normal-mapping](#-enabling_normal-mapping)
  * [Toon-shading](#-toon-shading)
    * [Enabling Toon-shading](#-enabling_toon-shading)
    * [Default Toon-shading](#-default_toon-shading)
    * [X-Toon shading](#-x-toon_shading)
  * [Modifying the model structure](#-modifying_the_model_structure)
    * [Reloading the 3D model](#-reloading_the_3d_model)
    * [Filtering](#-filtering)
    * [Simplificating](#-simplificating)
  * [Subsurface scattering (Work In Progress)](#-subsurface_scattering_(work_in_progress))
    * [Depth mapping](#-depth_mapping)
    * [Rendering the distance traveled by the light](#-rendering_the_distance_traveled_by_the_light)
    * [Subsurface scattering contribution](#-subsurface_scattering_contribution)
* [Building the project](#-building_the_project)
* [Running the project](#-running_the_project)

# Using<a name="-using"></a>

## Moving the 3D model

To move the 3D model, use the mouse:

* left click + move the mouse to rotate
* middle click + move the mouse to zoom-in or zoom-out
* right click + move the mouse to move the model over X and Y direction

## Physically-Based Rendering

PBR was implemented using GGX microfacet model. It uses material albedo parameters that can be imported from a texture and a number of lights that can be changed.

![Alt text](Images/PBR.png?raw=true "PBR with GGX microfacet model")

*PBR with GGX microfacet model*

### Changing the number of lights

To use more or less lights, use keyboard's up and down arrows. The lights are designed according to the 3-points lighting method.

![Alt text](Images/3_point_lighting.png?raw=true "3 point-lighting with key light, fill light and back light")

*3 point-lighting with key light, fill light and back light*

### Enabling texturing

To enable or disable teXturing, press the X key. By default, it textures the model with brick appearance.

![Alt text](Images/texturing.png?raw=true "Texturing with brick")

*Texturing with brick*

To add new textures, create a new subfolder in Resources/Material and add the texture files. The texture can be changed using the MATERIAL_NAME variable in Main.cpp. To do so, replace

```
static const std::string MATERIAL_NAME ("Brick/");
```

by

```
static const std::string MATERIAL_NAME ("Name/");
```

where Name is the name of any subfolder in Resources/Material.

### Enabling normal-mapping

To enable or disable Normal-mapping, press the N key. The texture used for normal-mapping is the one used in the Resources/Material/MATERIAL_NAME folder. 

![Alt text](Images/normal_mapping.png?raw=true "Normal mapping")

*Normal mapping*

![Alt text](Images/normal_mapping-textured.png?raw=true "Normal mapping with textured 3D model")

*Normal mapping with textured 3D model*

As the UV coordinates of the textures does not fit the 3D model, normal discontinuities can be perceived along the plane Z = 0.

![Alt text](Images/normal_mapping-discontinuity.png?raw=true "Normal mapping discontinuity")

*Normal mapping discontinuity*

## Toon-shading

While PBR aims to render images as photorealistic as possible, Toon-shading is a non-photorealistic rendering based on expressived styles.

### Enabling Toon-shading

To enable and disable Toon-shading, press the T key.

### Default Toon-shading

The default Toon-shading is a commonly used 1D texturing. 

![Alt text](Images/toon_shading.png?raw=true "Toon-shading")

*Toon-shading*

The colors:

* black for the edges
* white for highly specular areas (i.e. light is mainly reflected in the direction of the camera)
* green for the rest

Default shading mode can be recovered using the 1 numeric key.

### X-Toon shading

X-Toon shading is a method described in the paper X-Toon: An Extended Toon Shader written by Pascal Barla. Instead of providing a 1D texture to perform an expressive rendering, a 2D texture can be used. This way, the toon-shading is extended by a criteria. List of commonly used criteria:

* distance to the camera along camera axis (press the 2 numeric key)
* distance to the camera along focal axis (press the 3 numeric key)
* orientation of the surface with respect to the camera (press the 4 numeric key)

![Alt text](Images/x_toon_shading-depth.png?raw=true "X-Toon shading with a distance to the camera along camera axis criteria")

*X-Toon shading with a distance to the camera along camera axis criteria*

The resulting effect is that color changes are more sudden in the farthest regions than in the nearest ones. This way, less attention is paid to the silhouette. This effect only depends on the texture and the criteria.

## Modifying the model structure

In several applications, 3D models are not rendered in totality but modified before. For example, there is no need to render all the details of a far tree if only three pixels are affected by this model.

### Reloading the 3D model

Playing with the sections below will distord the model with no doubt.
To load the proper initial 3D model, press the F5 key.

### Filtering

A Laplacian filtering can be performed. The idea is to move vertices along their Laplacian to filter details. To perform a Laplacian filtering, press the I, O and P keys. Each key has an associated coefficient. The higher is the coefficient, the fewer is the number of iterations needed to filter the model. But the lower is the coefficient, the more precise is the filtering.

* I moves vertices from 0.1 of the Laplacian vector
* O moves vertices from the half of the Laplacian vector
* P moves vertices from the entire Laplacian vector

![Alt text](Images/filtering.png?raw=true "Laplacian filtering")

*Laplacian filtering*

### Simplificating

The 3D model can be simplified using clustering.

To Simplificate the model with a predefined resolution, press the S key.
To simplificate the model using an Adaptaive resolution implemented with an octree, press the A key.

![Alt text](Images/simplification.png?raw=true "Predefined simplification")


*Predefined simplification*

## Subsurface scattering (Work In Progress)

### Depth mapping

A shader is used to compute a depth map from the light point of view (for now, only the key light is considered) and write it in a texture using a framebuffer.

![Alt text](Images/depth_mapping.png?raw=true "Depth mapping with light point of view")

To visualize the result of the shader, press the 5 numeric key.

### Rendering the depth traveled by the light

A shader computes, for all fragments, the distance the light has traveled in the object. This is used to render the subsurface scattering effect.

![Alt text](Images/distance_traveled.png?raw=true "Distance traveled by light from the point it enters the object to the fragment"

### Subsurface scattering contribution

The Phong computation is augmented with the subsurface scattering contribution when pressing the B key. Pressing B key again display the subsurface scattering effect only.

![Alt text](Images/subsurface_scattering.png?raw=true "Result of the subsurface scattering, with light behind the object"

The subsurface scattering contribution is exponentially decreasing as the distance traveled by light increases.


# Building the project

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

# Running the project

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
