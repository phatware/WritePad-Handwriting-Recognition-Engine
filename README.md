
# WRITEPAD Handwriting Recognition Engine (HWRE)

Multilingual, multiplatfrom, self-contained handwriting recognition engine.

### About

This source code is distributed under the GPL v 3.0 license. In short, this means that ALL modifications and/or derivative works that use all or any portion of source code in the repository must be made public. 

If you would like to use this software for commercial purposes, you can purchase the commercial license from PhatWare Corp., which will supersede the GPL license. For detailed information about licensing, please contact **PhatWare Corp.** at developer@phatware.com

Compiled libraries and sample code are not included with this project. If you are looking for WritePad Handwriting Recognition SDK binaries to integrate with your commercial or a freeware project or for sample code for various platforms, visit the WritePad SDK repository at https://github.com/phatware/WritePadSDK

This repository contains the complete source code of WritePad Handwriting Recognition Engine (HWRE). It is a self-contained project, which does not require any 3rd party development tools or libraries. **Everything you need is here!** You can build WritePad HWRE for the following platforms:

* Apple MAC OS X 10.10+
* Xcode 8+
* Apple iOS 9+
* Microsoft Windows 7+ (Desktop and Windows Store DLLs; ARM, x64, i86)
* Microsoft Windows Mobile 8+
* Linux (practically any distribution with ISO C/C++ compiler)
* Google Android NDK, SDK 4+ (ARM, MIPS, MIPS64, ARM64, i64, i86)

### Building WritePad Engine

Building the handwriting recognition engine on different platforms

**1. MAC OS**

**Development Environment**: Xcode 8+

Open Xcode; open `MACOSX/WritePadSDK-OSX/WritePadSDK-OSX.xcodeproj` project; build for a desired target. This project will build a static library for MAC OS X.


**2. iOS**

**Development Environment**: Xcode 8+

Open Xcode; open `iOS/WritePadReco/WritePadReco.xcodeproj` project; build for a desired target. This project will build a static library for device or simulator depending on the chosen target.

**3. Android**

**Development Environment**: Android NDK 9+ (optional: Android SDK, Android Studio)

To build run `<ndk-path>/ndk-build` in the `Android/WritePadLib` folder. Android native dynamic (.so) libraries will be built for all supported CPUs.

**4. Linux**

**Development Environment**: Linux with C/C++ compiler 

To build simply run `make` in the `Linux` folder.

**5. Windows (desktop)**

**Development Environment**: Windows 10, Visual Studio 2015 Community Edition or better (older version(s) of Visual Studio can also be used, you may need to create a new solution workspace compatible with older version).

Start Visual Studio and open  `libuniversalreco.sln` in the `/UniversalRecognizer/V300` folder. Select the desired build configuration and platform then choose Build/Build Solution.

**6. Windows Store/Windows Mobile**

**Development Environment**: Windows 10, Visual Studio 2015 Community Edition or better (older version(s) of Visual Studio can also be used, you may need to create a new solution workspace compatible with older version).

Start Visual Studio and open  `WritePadReco.sln` in the `UniversalRecognizer/V300/WindowsStore/WindowsStoreReco` folder. Select the desired build configuration and platform, then choose Build/Build Solution.

### SDK Tools

Additonal information about building and using WritePad SDK tools will be posted here soon.


### Notes

_Developer Note_: We regret to inform you that most of the comments and internal development documentation were deemed confidential and have been excluded from this project. Some limited SDK documentation can be found in the Documentation folder.

