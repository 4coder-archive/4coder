@echo off

call "ctime" -begin 4ed_data.ctm

SET OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
SET OPTS=%OPTS% /GR- /EHa- /nologo /FC

SET FirstError=0

SET BUILD_MODE=%1
if "%BUILD_MODE%" == "" (SET BUILD_MODE="/DDEV_BUILD")

REM if "%BUILD_MODE%" == "/DDEV_BUILD_X86" (call "SETUP_CLX86")

pushd ..\build
cl %OPTS% ..\code\meta\build.cpp /Zi /Febuild %BUILD_MODE%
if %ERRORLEVEL% neq 0 (set FirstError=1)
if %ERRORLEVEL% neq 0 (goto END)
popd

..\build\build
if %ERRORLEVEL% neq 0 (set FirstError=1)

:END

REM if "%BUILD_MODE%" == "/DDEV_BUILD" (call "SETUP_CLX64")

call "ctime" -end 4ed_data.ctm %FirstError%
