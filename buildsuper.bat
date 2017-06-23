@echo off

REM This stores the path of the buildsuper.bat script
REM in CODE_HOME.  This way you can always include the
REM default files no matter where you store your code.
REM And no matter how you call buildsuper.bat.
SET CODE_HOME=%~dp0

IF NOT DEFINED LIB (call "%CODE_HOME%\\build_scripts\\setup_cl_x64.bat")

SET SRC=%1
if "%SRC%" == "" SET SRC=4coder_default_bindings.cpp

SET OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4457 /WX
SET OPTS=%OPTS% /GR- /nologo /FC
SET DEBUG=/Zi
SET BUILD_DLL=/LD /link /INCREMENTAL:NO /OPT:REF
SET EXPORTS=/EXPORT:get_bindings /EXPORT:get_alpha_4coder_version

cl %OPTS% /I"%CODE_HOME% " %DEBUG% "%SRC%" /Fecustom_4coder %BUILD_DLL% %EXPORTS%

REM file spammation preventation
del *.exp
del *.obj
del *.lib
