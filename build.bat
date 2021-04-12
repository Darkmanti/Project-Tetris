@echo off

IF "%1" EQU "/?" goto Help

set SrcFile="%cd%\src\unity_build.cpp"

set AddLib=User32.lib Shell32.lib Gdi32.lib Ole32.lib
set AddDll=

set BuildDir=build\
set BuildDirRe=%BuildDir%release\
set BuildDirDe=%BuildDir%debug\
set BuildDirHy=%BuildDir%hybrid\

set ReKeys=/O2
set DeKeys=/Zi /Od
set HyKeys=/Zi /O2

IF NOT EXIST %BuildDir% mkdir %BuildDir%
IF NOT EXIST %BuildDirRe% mkdir %BuildDirRe%
IF NOT EXIST %BuildDirDe% mkdir %BuildDirDe%
IF NOT EXIST %BuildDirHy% mkdir %BuildDirHy%

IF "%1" EQU "" goto All
IF /I "%1" EQU "re" goto Release
IF /I "%1" EQU "de" goto Debug
IF /I "%1" EQU "hy" goto Hybrid
IF /I "%1" EQU "clear" goto Clear

goto Exit

:Release
pushd %BuildDirRe%
cl %ReKeys% %SrcFile% %AddLib% %AddDll%
popd
goto Exit

:Debug
pushd %BuildDirDe%
cl %DeKeys% %SrcFile% %AddLib% %AddDll%
popd
goto Exit

:Hybrid
pushd %BuildDirHy%
cl %HyKeys% %SrcFile% %AddLib% %AddDll%
popd
goto Exit

:All
pushd %BuildDirRe%
cl %ReKeys% %SrcFile% %AddLib% %AddDll%
popd
pushd %BuildDirDe%
cl %DeKeys% %SrcFile% %AddLib% %AddDll%
popd
pushd %BuildDirHy%
cl %HyKeys% %SrcFile% %AddLib% %AddDll%
popd
goto Exit

:Clear
pushd %BuildDirRe%
del /Q *.*
popd
pushd %BuildDirDe%
del /Q *.*
popd
pushd %BuildDirHy%
del /Q *.*
popd
goto Exit

:Help
echo file have a 3 command line parameters "de" and "re" and "hy"
echo "de" - debug build without any optimization and with debug info
echo "hy" - hybrid build with all optimization and with debug info
echo "re" - release build with all optimization and without debug info
echo running the command with no parameters will execute all of these builds
echo "clear" cleaning all build directory
goto Exit

:Exit