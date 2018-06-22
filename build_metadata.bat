@echo off

set code_home=%~dp0
if %code_home:~-1%==\ (set code_home=%code_home:~0,-1%)

set src=%1
if "%src%" == "" set src=4coder_default_bindings.cpp

set opts=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
set opts=%opts% /GR- /EHa- /nologo /FC

pushd ..\build
set preproc_file=4coder_command_metadata.i
set meta_macros=/DMETA_PASS
cl /I"%code_home%" %opts% %debug% %code_home%\%src% /P /Fi%preproc_file% %meta_macros%
cl %opts% ..\code\4coder_metadata_generator.cpp /Zi /Femetadata_generator
metadata_generator -R "%code_home%" "%cd%\\%preproc_file%"

popd


