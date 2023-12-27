# 3D Model Integration
Jacob Yealy   
12/26/2023

## Table of contents:
- [Concept](#concept)
- [Progression](#progression)
- [Hierarchy](#hierarchy)
- [Use](#CMake)


### Concept
This part of the project involves integrating 3D models into our Vulkan architecture.  
The purpose of the chosen models are to mock dueling dragons around a planet.

### Hierarchy
To create an effective scene, a hierarchy was established for complex objects. 
Six planets are arranged around the dragons. The dragons are made parent objects of the planets.
The purpose of establishing parent objects is so that when a parent object moves, the child object moves
in conjunction. This scale and animation style can be edited from parent to child object.

## Progression

![progress.png](textures%2Fprogress.png)


# CMake
This project is configured for use in a Linux environment. If using another OS, simply edit the CMake file.

### References
- Little Vulkan Engine Tutorial: https://www.youtube.com/watch?v=uZqxj6tLDY4&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=31
- 3D model 1 (Dragon): https://www.turbosquid.com/3d-models/hinh-xam-chinese-dragon-paint-lowpoly-rigged-3d-model-1946573
- 3D model 2 (Planet): https://www.turbosquid.com/3d-models/3d-model-realistic-venus-photorealistic-1k-1305695
- 3D model 3 (Sky): https://www.turbosquid.com/3d-models/utah-desert-terrain-model-1982164
