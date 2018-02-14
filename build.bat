@echo off

SET OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
SET OPTS=%OPTS% /GR- /EHa- /nologo /FC

SET FirstError=0

SET BUILD_MODE=%1
if "%BUILD_MODE%" == "" (SET BUILD_MODE="/DDEV_BUILD")

pushd ..\build
cl %OPTS% kernel32.lib ..\code\meta\4ed_build.cpp /Zi /Febuild %BUILD_MODE%
if %ERRORLEVEL% neq 0 (set FirstError=1)
if %ERRORLEVEL% neq 0 (goto END)
popd

..\build\build
:END
if %ERRORLEVEL% neq 0 (set FirstError=1)

