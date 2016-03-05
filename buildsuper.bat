@echo off

REM this is here to prevent the spammation of PATH
IF NOT DEFINED clset (call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" x64)
SET clset=64

SET WARNINGS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /WX
SET STUFF=/GR- /nologo
SET DEBUG=/Zi
SET EXPORTS=/EXPORT:get_bindings
SET SRC=4coder_custom.cpp
SET LINKS=user32.lib gdi32.lib
REM SET LINKS=

cl %WARNINGS% %STUFF% %DEBUG% %SRC% %LINKS% /Fe4coder_custom /LD /link /INCREMENTAL:NO /OPT:REF %EXPORTS%

REM file spammation preventation
del *.exp
del *.obj
del *.lib
