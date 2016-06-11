@echo off

REM this is here to prevent the spammation of PATH
IF NOT DEFINED clset (call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64)
SET clset=64

SET SRC=%1
if "%SRC%" == "" SET SRC=4coder_default_bindings.cpp

SET OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /WX
SET OPTS=%OPTS% /GR- /nologo
SET DEBUG=/Zi
set BUILD_DLL=/LD /link /INCREMENTAL:NO /OPT:REF
SET EXPORTS=/EXPORT:get_bindings /EXPORT:get_alpha_4coder_version

REM SET LINKS=user32.lib gdi32.lib
SET LINKS=

REM This stores the path of the buildsuper.bat script
REM in CODE_HOME.  This way you can always include the
REM default files no matter where you store your code.
REM And no matter how you call buildsuper.bat.
SET CODE_HOME=%~dp0

cl /I%CODE_HOME% %OPTS% %DEBUG% %SRC% %LINKS% /Fe4coder_custom %BUILD_DLL% %EXPORTS%

REM file spammation preventation
del *.exp
del *.obj
del *.lib
