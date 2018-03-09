@echo off

REM This stores the path of the buildsuper.bat script
REM in CODE_HOME.  This way you can always include the
REM default files no matter where you store your code.
REM And no matter how you call buildsuper.bat.
set code_home=%~dp0
if %code_home:~-1%==\ (set code_home=%code_home:~0,-1%)

if NOT "%Platform%" == "X86" IF NOT "%Platform%" == "x86" (call "%code_home%\windows_scripts\setup_cl_x86.bat")

set SRC=%1
if "%SRC%" == "" set SRC=4coder_default_bindings.cpp

set OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4457 /WX
set OPTS=%OPTS% /GR- /nologo /FC
set DEBUG=/Zi
set BUILD_DLL=/LD /link /INCREMENTAL:NO /OPT:REF
set EXPORTS=/EXPORT:get_bindings /EXPORT:get_alpha_4coder_version

cl %OPTS% /I"%code_home%" %DEBUG% "%code_home%\4coder_metadata_generator.cpp" /Femetadata_generator
metadata_generator -R "%code_home%" "%code_home%\*"

cl %OPTS% /I"%code_home%" %DEBUG% "%SRC%" /Fecustom_4coder %BUILD_DLL% %EXPORTS%

REM file spammation preventation
del metadata_generator*
del *.exp
del *.obj
del *.lib
