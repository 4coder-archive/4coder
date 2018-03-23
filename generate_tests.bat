@echo off

if not exist ..\tests (mkdir ..\tests)
if not exist ..\tests\input_data (mkdir ..\tests\input_data)

set code=%cd%
pushd ..\build
set build=%cd%
popd

set name=test_generator
set full_name=%build%\%name%
set scripts=%code%\test_input_scripts\generated

set opts=
set opts=%opts% /W4 /WX /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390
set opts=%opts% /GR- /EHa- /nologo /FC

set inc=-I%code%

pushd %build%
cl %opts% %inc% %code%\meta\4ed_test_generator.cpp /Zi /Fe%name%
popd

pushd %scripts%
%full_name%
popd
