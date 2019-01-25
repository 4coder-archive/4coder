@echo off

set opts=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
set opts=%opts% /GR- /EHa- /nologo /FC

set FirstError=0

set build_mode=%1
if "%build_mode%" == "" (set build_mode="/DDEV_BUILD")

pushd ..\build
call cl %opts% kernel32.lib ..\code\meta\4ed_build.cpp /Zi /Febuild %build_mode%
if %ERRORLEVEL% neq 0 (set FirstError=1)
if %ERRORLEVEL% neq 0 (goto END)
popd

..\build\build
:END
if %ERRORLEVEL% neq 0 (set FirstError=1)

