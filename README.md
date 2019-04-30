# TF2Vulkan
A [Vulkan](https://www.khronos.org/vulkan/) rendering backend for [Team Fortress 2](http://www.teamfortress.com/).

## Current Status:
Working on reimplementing shaderapidx9.dll with one that uses Vulkan. Eventually, materialsystem.dll will also probably be replaced, since I believe there's lots of room for optimization there as well.

### Before anyone gets too excited:
* This will likely never be usable without -insecure (you won't be able to play on VAC-secured servers). 
* If you get terrible FPS in TF2, your graphics hardware is likely so old that your drivers don't support Vulkan anyway.

### Why?
* Yes.
