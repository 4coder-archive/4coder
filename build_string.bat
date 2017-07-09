@echo off

build.bat /DSTRING_BUILD

REM SET OPTS=-W4 -wd4310 -wd4100 -wd4201 -wd4505 -wd4996 -wd4127 -wd4510 -wd4512 -wd4610 -wd4390 -WX
REM SET OPTS=%OPTS% -wd4611 -GR- -EHa- -nologo -FC
REM 
REM SET FirstError=0
REM 
REM pushd ..\build
REM cl %OPTS% ..\code\string\4ed_string_builder.cpp /Zi /Festring_builder
REM if %ERRORLEVEL% neq 0 (set FirstError=1)
REM if %ERRORLEVEL% neq 0 (goto END)
REM popd
REM 
REM pushd string
REM ..\..\build\string_builder
REM if %ERRORLEVEL% neq 0 (set FirstError=1)
REM popd
REM 
REM :END
