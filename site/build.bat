@echo off

call "ctime" -begin 4ed_site.ctm

SET OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
SET OPTS=%OPTS% /GR- /EHa- /nologo /FC

SET FirstError=0

pushd ..\..\build
cl %OPTS% ..\code\build.cpp /Zi /Febuild /DSITE_BUILD
if %ERRORLEVEL% neq 0 (set FirstError=1)
popd

pushd ..
..\build\build
if %ERRORLEVEL% neq 0 (set FirstError=1)
popd

call "ctime" -end 4ed_site.ctm %FirstError%