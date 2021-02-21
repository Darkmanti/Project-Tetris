@echo off
set vcvarsallDir="C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat"

call %vcvarsallDir% x64

cd ..\
echo.
echo Run the "build.bat" file to build the project
echo ======================================================================
CMD /K