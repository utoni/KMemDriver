@echo off

set PDSECTRL="%~dp0\..\PastDSE\x64\Debug\PastDSECtrl.exe"
if not exist %PDSECTRL% set PDSECTRL="%~dp0\..\PastDSE\bin\x64\Debug\PastDSECtrl.exe"

call %~dp0\..\PastDSE\driver-start.bat
%PDSECTRL% %~dp0\x64\Debug\KMemDriver.sys
call %~dp0\..\PastDSE\driver-stop.bat
REM fsutil usn deleteJournal /D C:

REM set /p wndtitle="Enter Window Title: "
REM IF [%wndtitle%] == [] (%~dp0\x64\Release\KTest.exe) ELSE (%~dp0\x64\Release\KTest.exe wnd "%wndtitle%")
timeout /t 3