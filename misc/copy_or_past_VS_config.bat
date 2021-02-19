@echo off
set miscSrcDir="%cd%\VS_config\src\"
pushd ..\src\
IF NOT EXIST src.sln ( XCOPY %miscSrcDir%src.sln /Y "%cd%") ELSE ( XCOPY src.sln /Y %miscSrcDir%)
IF NOT EXIST src.vcxproj ( XCOPY %miscSrcDir%src.vcxproj /Y "%cd%") ELSE ( XCOPY src.vcxproj /Y %miscSrcDir%)
IF NOT EXIST src.vcxproj.filters ( XCOPY %miscSrcDir%src.vcxproj.filters /Y "%cd%") ELSE ( XCOPY src.vcxproj.filters /Y %miscSrcDir%)
IF NOT EXIST src.vcxproj.user ( XCOPY %miscSrcDir%src.vcxproj.user /Y "%cd%") ELSE ( XCOPY src.vcxproj.user /Y %miscSrcDir%)
popd