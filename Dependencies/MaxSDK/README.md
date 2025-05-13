# 3D Studio Max SDK

The 3D Studio Max SDK is required to build the W3D plugin. Currently this plugin is only for an old version of 3DS Max and as such should be considered for reference only. The SDK license is incompatible with GPLv3 so we do not supply the SDK and do not distribute the plugin that can be built.

If you wish to enable building the plugin locally, you can do so by providing the 4.2.0 Max SDK in this folder. It can be obtained from The Internet Archive [here](https://archive.org/details/maxsdk-4.2.0.85). Download the zip and unzip it to this folder. This should result in this folder containing this file (README.md), the CMake build file providing configuration information to use the SDK (CMakeLists.txt) and the SDK folder extracted from the zip (maxsdk). When you build the project, provided building development tools is enabled, a max plugin "max2w3d.dle" will be produced.
