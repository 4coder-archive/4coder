@echo off

if not exist ..\tests (mkdir ..\tests)
if not exist ..\tests\input_data (mkdir ..\tests\input_data)

set code=%cd%
pushd ..\build
set build=%cd%
popd
pushd ..\4coder-non-source\test_data\input_data
set data=%cd%
popd

set name=test_builder
set full_name=%build%\%name%
set scripts=%code%\test_input_scripts

set opts=
set opts=%opts% /W4 /WX /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390
set opts=%opts% /GR- /EHa- /nologo /FC

set inc=-I%code%

pushd %build%
cl %opts% %inc% %code%\meta\4ed_test_builder.cpp /Zi /Fe%name%
popd

pushd %data%
%full_name% %scripts%\test_full_click.4is
%full_name% %scripts%\test_write_4coder_awesomeness.4is
%full_name% %scripts%\test_bootstrap.4is
popd