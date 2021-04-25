@echo off

IF "%1" EQU "/?" goto Help

set SrcFilePath="%cd%\src\unity_build.cpp"
set MainDir="%cd%"
set GameName=Tetris
set GameNameExe=unity_build.exe

set AddLib=User32.lib Shell32.lib Gdi32.lib Ole32.lib
set AddDll=

rem /D and without spaces write a definitions
set AllDef=
set DebDef=
set RelDef=
set HybDef=

set BuildDir=build\
set BuildDirRe=%BuildDir%release\
set BuildDirDe=%BuildDir%debug\
set BuildDirHy=%BuildDir%hybrid\
set GameDir=%BuildDir%%GameName%\

set ReKeys=/O2
set DeKeys=/Zi /Od
set HyKeys=/Zi /O2

IF NOT EXIST %BuildDir% mkdir %BuildDir%
IF NOT EXIST %BuildDirRe% mkdir %BuildDirRe%
IF NOT EXIST %BuildDirDe% mkdir %BuildDirDe%
IF NOT EXIST %BuildDirHy% mkdir %BuildDirHy%
IF NOT EXIST %GameDir% mkdir %GameDir%

xcopy /I /Y /E /Q %MainDir%\res %BuildDir%res

IF "%1" EQU "" goto All
IF /I "%1" EQU "re" goto Release
IF /I "%1" EQU "de" goto Debug
IF /I "%1" EQU "hy" goto Hybrid
IF /I "%1" EQU "clear" goto Clear
IF /I "%1" EQU "makegame" goto MakeGame

goto Exit

:Release
pushd %BuildDirRe%
cl %AllDef% %RelDef% %ReKeys% %SrcFilePath% %AddLib% %AddDll%
popd
goto Exit

:Debug
pushd %BuildDirDe%
cl %AllDef% %DebDef% %DeKeys% %SrcFilePath% %AddLib% %AddDll%
popd
goto Exit

:Hybrid
pushd %BuildDirHy%
cl %AllDef% %HybDef% %HyKeys% %SrcFilePath% %AddLib% %AddDll%
popd
goto Exit

:All
pushd %BuildDirRe%
cl %AllDef% %RelDef% %ReKeys% %SrcFilePath% %AddLib% %AddDll%
popd
pushd %BuildDirDe%
cl %AllDef% %DebDef% %DeKeys% %SrcFilePath% %AddLib% %AddDll%
popd
pushd %BuildDirHy%
cl %AllDef% %HybDef% %HyKeys% %SrcFilePath% %AddLib% %AddDll%
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

:MakeGame
echo D|xcopy /I /Y /E /Q %BuildDirRe%%GameNameExe% %GameDir%bin
xcopy /I /Y /E /Q %MainDir%\res %GameDir%\res
goto Exit

:Exit