# TF2Vulkan
A [Vulkan](https://www.khronos.org/vulkan/) rendering backend for [Team Fortress 2](http://www.teamfortress.com/).

## Current Status:
DISCLAIMER FOR ANYONE READING THIS IMMEDIATELY AFTER IT BECAME PUBLIC ON 10/10/2019: There are currently "device lost" errors that I have not had time to resolve due to extreme life situations and lots of schoolwork. 

Working on reimplementing shaderapidx9.dll with one that uses Vulkan. Eventually, materialsystem.dll will also probably be replaced, since I believe there's lots of room for optimization there as well.

Shaders currently only support textures and lightmapping, and other effects such as environment maps/phong/etc still need to be implemented in the unified Vulkan shaders.

### Before anyone gets too excited:
* This will likely never be usable without -insecure (you won't be able to play on VAC-secured servers). 
* If you get terrible FPS in TF2, your graphics hardware is likely so old that your drivers don't support Vulkan anyway.

### Why?
* Learning and challenge!
