@echo off

if not exist ..\tests (mkdir ..\tests)
if not exist ..\tests\input_data (mkdir ..\tests\input_data)

set code_home=%cd%
pushd ..\build
set build_home=%cd%
popd
pushd ..\tests\input_data
set data_home=%cd%
popd

set opts=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
set opts=%opts% /GR- /EHa- /nologo /FC

set inc=-I%code_home%

pushd %build_home%
cl %opts% %inc% %code_home%\meta\4ed_test_builder.cpp /Zi /Fetest_builder
popd

pushd %data_home%
%build_home%\test_builder %code_home%\test_scripts\test_full_click.4is
%build_home%\test_builder %code_home%\test_scripts\test_write_4coder_awesomeness.4is
%build_home%\test_builder %code_home%\test_scripts\test_bootstrap.4is
popd