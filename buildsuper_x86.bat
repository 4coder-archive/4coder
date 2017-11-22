@echo off

REM This stores the path of the buildsuper.bat script
REM in CODE_HOME.  This way you can always include the
REM default files no matter where you store your code.
REM And no matter how you call buildsuper.bat.
SET CODE_HOME=%~dp0

IF NOT "%Platform%" == "X86" IF NOT "%Platform%" == "x86" (call "%CODE_HOME%\\windows_scripts\\setup_cl_x86.bat")

SET SRC=%1
if "%SRC%" == "" SET SRC=4coder_default_bindings.cpp

SET OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4457 /WX
SET OPTS=%OPTS% /GR- /nologo /FC
SET DEBUG=/Zi
SET BUILD_DLL=/LD /link /INCREMENTAL:NO /OPT:REF
SET EXPORTS=/EXPORT:get_bindings /EXPORT:get_alpha_4coder_version

cl %OPTS% /I"%CODE_HOME% " %DEBUG% "%CODE_HOME%4coder_metadata_generator.cpp" /Femetadata_generator
metadata_generator -R "%CODE_HOME% " "%CODE_HOME% "

cl %OPTS% /I"%CODE_HOME% " %DEBUG% "%SRC%" /Fecustom_4coder %BUILD_DLL% %EXPORTS%

REM file spammation preventation
del metadata_generator*
del *.exp
del *.obj
del *.lib
