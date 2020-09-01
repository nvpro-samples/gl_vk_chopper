# gl_vk_chopper

Simple Vulkan rendering example.

![Sample screenshot: Helicopters fly through the sky above an ocean.](https://raw.githubusercontent.com/nvpro-samples/gl_vk_chopper/master/doc/chopper.png)

This sets up the Vulkan Device, queue, etc., loads a model from a bespoke file format along with associated materials and textures, and renders with a single thread.

## Building

To build this sample, first install the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/). Then do one of the following:

* To clone all NVIDIA DesignWorks Samples, clone https://github.com/nvpro-samples/build_all, then run one of the `clone_all` scripts in that directory.
* Or to get the files for this sample without the other samples, clone this repository as well as https://github.com/nvpro-samples/shared_sources into a single directory. On Windows, you'll need to clone https://github.com/nvpro-samples/shared_external into the directory as well.

You can then use CMake to generate and subsequently build the project.
