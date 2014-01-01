
REM root directory
cd ..\..
del Makefile

REM .\VersionTool directory
cd VersionTool

del Makefile
del Makefile.Debug
del Makefile.Release
del vc110.pdb

REM remove debug and release folders including their content
del /s /q .\debug\*
rmdir /s /q .\debug\
del /s /q .\release\*
rmdir /s /q .\release\

REM .\Quazaa directory
cd ../Quazaa

del Makefile
del Makefile.Debug
del Makefile.Release
del object_script.Quazaa.Release
del object_script.Quazaa_debug.Debug
del vc110.pdb
del version.h

REM remove temp directory
del /s /q .\temp\*
rmdir /s /q .\temp\

REM remove debug and release folders including their content
del /s /q .\debug\*
rmdir /s /q .\debug\
del /s /q .\release\*
rmdir /s /q .\release\

REM .\UnitTests directory
cd ../UnitTests

del Makefile
del Makefile.Debug
del Makefile.Release

REM remove temp directory
del /s /q .\temp\*
rmdir /s /q .\temp\

REM remove debug and release folders including their content
del /s /q .\debug\*
rmdir /s /q .\debug\
del /s /q .\release\*
rmdir /s /q .\release\

@echo off
echo.
echo.
echo Hit a random key to finish script.
pause