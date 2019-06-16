# What?
*KMemDriver* is a **Windows 10 x64 driver** designed to manipulate memory (and more)
from ring0. It is also possible to bypass existing ring0/ring3 AntiCheat solutions e.g. BE and EAC.


# Dependencies
- Visual Studio 2017 Community Edition
- Windows 10 x64 1803 (may work on older versions, not verified)
- Windows 10 SDK 10.0.17763.0
- Windows Driver Kit
- Windows Universal CRT SDK
- C++/CLI support
- VC++ 2017 tools

The recommended way to install all dependencies is through [vs_community.exe](https://visualstudio.microsoft.com/).


# HowTo
*KMemDriver* was designed work together with *PastDSE* as injector.
*KMemDriver* supports manual mapping.


# Contributors
As you can see, I've used some slightly modified code from [BlackBone](https://github.com/DarthTon/Blackbone) for VAD routines.
