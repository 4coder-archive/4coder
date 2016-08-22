@echo off

set WARNINGOPS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
set WARNINGOPS=%WARNINGOPS% /GR- /EHa- /nologo /FC
set WIN_LIBS=user32.lib winmm.lib gdi32.lib

pushd w:\filetrack\build
cl %WARNINGOPS% ..\code\filetrack_test.c /Fefile_rewriter /Zi %*
cl %WARNINGOPS% ..\code\filetrack_main.c /Fefiletrack /Zi %*
popd