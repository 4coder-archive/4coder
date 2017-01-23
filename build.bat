@echo off

call "ctime" -begin 4ed_data.ctm

SET OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
SET OPTS=%OPTS% /GR- /EHa- /nologo /FC

SET FirstError=0

SET BUILD_MODE=%1
if "%BUILD_MODE%" == "" (SET BUILD_MODE="/DDEV_BUILD")

pushd ..\build
cl %OPTS% ..\code\meta\build.cpp /Zi /Febuild %BUILD_MODE%
if %ERRORLEVEL% neq 0 (set FirstError=1)
if %ERRORLEVEL% neq 0 (goto END)
popd

..\build\build
if %ERRORLEVEL% neq 0 (set FirstError=1)

pushd ..\build
call "print_size.bat" 4coder_custom.dll
call "print_size.bat" 4ed_app.dll
call "print_size.bat" 4ed.exe
popd

:END
call "ctime" -end 4ed_data.ctm %FirstError%
