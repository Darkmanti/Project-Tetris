@echo off
echo *******************************************************************************************
echo **       This is the script to search for the file "vcvarsall.bat" on this computer      **
echo **You can see the search result in the file "pathtovcvarsall.txt" in the script directory**
echo *******************************************************************************************
echo Wait for the script to complete the search...
For /F "tokens=*" %%i in ('where /r C:\ vcvarsall.bat') do @echo %%i>pathto_vcvarsall.txt