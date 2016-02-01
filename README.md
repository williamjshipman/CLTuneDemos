# CLTuneDemos
Demo programs showing how to use [CLTune](https://github.com/CNugteren/CLTune) for autotuning OpenCL kernels.

At present, the only demo is a median filter but I hope to add more.

## Building the demos

You'll need to download the Eclipse CDT IDE, MinGW-w64 and an OpenCL SDK in order to build this demo.
You will need to update the paths for the OpenCL SDK include and library folders.
I'm still looking for a good way to make this repository less dependent on Eclipse CDT, but unfortunately the makefiles are all in the Debug or Release folder.

## TODO

- Add more demos.
- Create platform and tool-chain independent projects (perhaps CMake projects in Eclipse CDT).