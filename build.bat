@echo off

set BuildDirSrc=build\

IF NOT EXIST %BuildDirSrc% mkdir %BuildDirSrc%

pushd %BuildDirSrc%
cl /O2 ..\src\main.cpp
popd