# What?
*KMemDriver* is a **Windows 10 x64 driver** designed to manipulate memory (and more)
from ring0. It is also possible to bypass existing ring0/ring3 AntiCheat solutions e.g. BE and EAC.
It can also be used to manual map a user space DLL to a protected process.
Communication is done through virtual memory.


# Dependencies
- Visual Studio 2017 Community Edition
- Visual C++ MFC for x86 and x64
- Windows 10 x64 1803 (may work on older versions, not verified)
- Windows 10 SDK 10.0.17763.0
- Windows Driver Kit 1803 (10.0.17763.0)
- Windows Universal CRT SDK
- C++/CLI support
- VC++ 2017 tools

The recommended way to install all dependencies is through [vs_community.exe](https://visualstudio.microsoft.com/).


# HowTo
*KMemDriver* was designed work together with *PastDSE* as injector.
*KMemDriver* supports manual mapping in terms as it does not use any kernel symbol (with 1 exception) that require a legit loaded driver.

For the use with *PastDSE*:
Make sure that *KMemDriver* and *PastDSE* are in the same folder.
With that you can just inject by executing PastDSE-Manual-Map-\*.bat as Administrator.


# Tests
To make sure that KMemDriver works as expected you can run an integration test.


# Features
- communicates to the user space controller program via a shared memory alike mechanism
- uses (unnamed) Windows events for kernel space and user space as synchronization
- read all mapped memory pages of a process
- read all mapped modules of process
- read memory of a process (bypass page protections)
- write memory to a process (bypass page protections)
- allocate memory with specified page protection to a process
- free memory of a process
- unlink memory from VAD of a process (requires PatchGuard to be disabled)


# AntiCheat status

I've used it only for EAC protected games.
If you are using only RPM and WPM (e.g. don't inject any DLL into an EAC protected processes) it may (or may not, depending on the game) more or less safe.
But keep in mind that manual mapped drivers (that start a system thread) and windows events (leaving traces in form of handles) is a detection vector that does not even require advanced anti cheat software for successful detection.
Injecting DLL's into EAC protected processes (w/o other countermeasures) will get you banned very soon.


# Contributors
As you can see, I've used some slightly modified code from [BlackBone](https://github.com/DarthTon/Blackbone) for VAD routines and manual DLL mapping.
