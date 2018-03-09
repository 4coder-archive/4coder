@echo off

set OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
set OPTS=%OPTS% /GR- /EHa- /nologo /FC

pushd ..\build
cl %OPTS% ..\code\4coder_metadata_generator.cpp /Zi /Femetadata_generator
popd

set code_home=%~dp0
if %code_home:~-1%==\ (set code_home=%code_home:~0,-1%)
..\build\metadata_generator -R "%code_home%" "%code_home%\*"


