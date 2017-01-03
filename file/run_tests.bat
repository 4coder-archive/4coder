@echo off

SET OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
SET OPTS=%OPTS% /GR- /EHa- /nologo /FC

pushd ..\build
cl %OPTS% ..\code\file\4coder_file_tests.cpp /Zi /Fefile_test
popd

if %ERRORLEVEL% eq 0 (..\build\file_test)

