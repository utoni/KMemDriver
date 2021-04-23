# What?
*KMemDriver* is a **Windows 10 x64 driver** designed to manipulate memory from ring0.
It can also be used to manual map a user space DLL to a protected process.
Communication is done through shared memory.


# Dependencies
- Visual Studio 2017/2019 Community Edition
- Visual C++ MFC for x86 and x64
- Windows 10 x64
- Windows 10 SDK (>=10.0.17763.0)
- Windows Driver Kit 1803 (>=10.0.17763.0)
- Windows Universal CRT SDK
- C++/CLI support
- VC++ 2017/2019 tools

The recommended way to install all dependencies is through [vs_community.exe](https://visualstudio.microsoft.com/).


# HowTo
*KMemDriver* was designed work together with *PastDSE* as injector.
*KMemDriver* supports manual mapping in terms as it does not use any kernel API that requires a legit loaded driver.

For the use with *PastDSE*:
Make sure that *KMemDriver* and *PastDSE* have the same parent folder.
With that you can just inject by executing PastDSE-Manual-Map-\*.bat as Administrator.

Remember that the driver detects possible clients by their basename suffix "kmem" e.g. `some-fancy-app-kmem.exe`
and waits until it maps the memory pages into it's address space (and responds to the handshake via mentioned shared memory).
Together with a kernel EVENT and user EVENT this builds the shared memory w/ bidirectional synchronisation.
Running multiple clients is not supported.


# Tests
To make sure that KMemDriver works as expected you can run an integration test after injection.


# Features
- communicates to the user space controller program via a shared memory alike mechanism
- uses (unnamed) Windows events for kernel space and user space as synchronization
- read all mapped memory pages of a process
- read all mapped modules of process
- read memory of a process (bypass page protections)
- write memory to a process (bypass page protections)
- allocate memory with specified page protection to a process
- free memory of a process


# AntiCheat status

I've used it for EAC protected games only.
If you are using only RPM and WPM (e.g. don't inject any DLL into an EAC protected processes) it may (or may not, depending on the game) not get detected (at the time of writing this).
But keep in mind that manual mapped drivers (e.g. that start a system thread) and windows events (leaving traces in form of handles) is a detection vector that does not even require advanced anti cheat software for successful detection.
Injecting DLL's into EAC protected processes (w/o other countermeasures) will get you banned very soon.


# Contributors
As you may see, I've used some slightly modified code from [BlackBone](https://github.com/DarthTon/Blackbone) for manual DLL mapping.
