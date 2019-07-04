# Ray

![Example image](https://raw.githubusercontent.com/sunverwerth/ray/master/examples/ray.png "Example trace")


Ray is a CPU path tracer written in c++

## Used libraries

* [SDL2](https://www.libsdl.org/index.php)
* [stb_image](https://github.com/nothings/stb)
* [tiny_obj_loader](https://github.com/syoyo/tinyobjloader)

## Features

* Multithreaded rendering
* Explicit area light sampling
* Depth of field
* Cosine weighted hemisphere sampling
* Russian roulette path termination
* Interactive controls
* Objects
    * Cubes
    * Spheres
    * Planes
    * Quads
    * Triangle meshes (.obj and Quake2 BSP)
    * RGBE Environment maps
* Material system
    * base color
    * emissive color
    * roughness
    * opacity
    * metallicity
    * index of refraction

## Controls

* Click and drag to pan camera
* Arrow keys to move camera
* Scroll wheel to adjust aperture size
* Click to set focal plane
* +/- to adjust exposure

## More examples

![Example image](https://raw.githubusercontent.com/sunverwerth/ray/master/examples/trace11.png "Example trace")

![Example image](https://raw.githubusercontent.com/sunverwerth/ray/master/examples/trace5.png "Example trace")

![Example image](https://raw.githubusercontent.com/sunverwerth/ray/master/examples/trace8.png "Example trace")
