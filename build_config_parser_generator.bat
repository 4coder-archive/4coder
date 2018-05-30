@echo off

set opts=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
set opts=%opts% /GR- /EHa- /nologo /FC
set opts=%opts% /Zi

pushd ..\build
cl %opts% ..\code\meta\4ed_meta_generate_parser.cpp /Fegenerate_config_parser
popd

