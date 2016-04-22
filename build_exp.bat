@echo off

set WARNINGOPS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
set WARNINGOPS=%WARNINGOPS% /GR- /EHa- /nologo /FC

pushd ..\build
cl %WARNINGOPS% ..\code\test\fsm_table_generator.cpp /Fefsm_gen %*

pushd ..\code\test
..\build\fsm_gen
popd

cl %WARNINGOPS% ..\code\test\experiment.cpp /Fexperiment %*
popd
