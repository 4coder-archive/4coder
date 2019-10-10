@echo off

REM This stores the path of the buildsuper.bat script
REM in CODE_HOME.  This way you can always include the
REM default files no matter where you store your code.
REM And no matter how you call buildsuper.bat.
set code_home=%~dp0
if %code_home:~-1%==\ (set code_home=%code_home:~0,-1%)

if NOT "%Platform%" == "X86" IF NOT "%Platform%" == "x86" (call "%code_home%\windows_scripts\setup_cl_x86.bat")

set src=%1
if "%src%" == "" set src=4coder_default_bindings.cpp

set opts=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4457 /WX
set opts=%opts% /GR- /nologo /FC
set debug=/Zi
set release=/O2 /Zi
set build_dll=/LD /link /INCREMENTAL:NO /OPT:REF
set build_dll=%build_dll% /EXPORT:get_version /EXPORT:init_apis

set mode=%debug%
if "%2" == "release" (set mode=%release%)

set preproc_file=4coder_command_metadata.i
set meta_macros=/DMETA_PASS
cl /I"%code_home%" %opts% %mode% %src% /P /Fi%preproc_file% %meta_macros%
cl /I"%code_home%" %opts% %mode% "%code_home%\4coder_metadata_generator.cpp" /Femetadata_generator
metadata_generator -R "%code_home%" "%cd%\\%preproc_file%"

cl %opts% /I"%code_home%" %mode% "%src%" /Fecustom_4coder %build_dll% %exports%

REM file spammation preventation
del metadata_generator*
del *.exp
del *.obj
del *.lib
del %preproc_file%
