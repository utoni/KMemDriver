# What?
*KMemDriver* is a **Windows 10 x64 driver** designed to manipulate memory (and more)
from ring0. It is also possible to bypass existing ring0/ring3 AntiCheat solutions e.g. BE and EAC.
It can also be used to manual map a user space DLL to a protected process and hide its occupied memory pages.


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
*KMemDriver* supports manual mapping in terms as it does not use any kernel symbol (with 1 exception) that require a legit loaded driver.


# Tests
To make sure that KMemDriver works as expected you can run two different kind of tests to verify it for your OS.
There are two different kind of tests:
- integration test (TODO)
- stress test (TODO)


# Features
- communicates to the user space controller program via own written shared memory alike mechanism
- uses Windows events for the kernel space and user space as synchronization
- read all mapped memory pages of a process
- read all mapped modules of process
- read memory of a process (bypass page protections)
- write memory to a process (bypass page protections)
- allocate memory with specified page protection to a process
- free memory of a process
- unlink memory from VAD of a process


# Contributors
As you can see, I've used some slightly modified code from [BlackBone](https://github.com/DarthTon/Blackbone) for VAD routines and manual DLL mapping.
