@echo off

set opts=-FC -GR- -EHa- -nologo -Zi
set code=%cd%
pushd ..\..\build
cl %opts% %code%\test_preproc_main.cpp -Fetest_preproc
popd
