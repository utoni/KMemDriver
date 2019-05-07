@echo off

REM fsutil usn deleteJournal /D C:
%~dp0\x64\Release\KTest.exe
timeout /t 3