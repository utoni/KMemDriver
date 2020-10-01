@echo off

REM fsutil usn deleteJournal /D C:
%~dp0\x64\Release\IntegrationTest-kmem.exe
timeout /t 3