@echo off
REM TODO: to third procedure and this file will get 3 parameters

IF "%1" EQU "/?" goto Help

set SrcFile="%cd%\src\main.cpp"
set BuildDir=build\
set BuildDirRe=%BuildDir%release\
set BuildDirDe=%BuildDir%debug\
set BuildDirHy=%BuildDir%hybrid\

IF NOT EXIST %BuildDir% mkdir %BuildDir%
IF NOT EXIST %BuildDirRe% mkdir %BuildDirRe%
IF NOT EXIST %BuildDirDe% mkdir %BuildDirDe%
IF NOT EXIST %BuildDirHy% mkdir %BuildDirHy%

IF "%1" EQU "" goto Release
IF /I "%1" EQU "re" goto Release
IF /I "%1" EQU "de" goto Debug
IF /I "%1" EQU "hy" goto Hybrid

goto Exit

:Release
pushd %BuildDirRe%
cl /O2 %SrcFile%
popd
goto Exit

:Debug
pushd %BuildDirDe%
cl /Zi /Od %SrcFile%
popd
goto Exit

:Hybrid
pushd %BuildDirHy%
cl /Zi /O2 %SrcFile%
popd
goto Exit

:Help
echo file have a 3 command line parameters "de" and "re" and "hy"
echo "de" - debug build without any optimization and with debug info
echo "hy" - hybrid build with all optimization and with debug info
echo "re" - release build with all optimization and without debug info
echo running the command with no parameters will execute release build
goto Exit

:Exit