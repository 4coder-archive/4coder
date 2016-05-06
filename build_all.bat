@echo off

set OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
set OPTS=%OPTS% /GR- /EHa- /nologo /FC
set INCLUDES=/I..\foreign
set LIBS=user32.lib winmm.lib gdi32.lib opengl32.lib
set ICON=..\res\icon.res
set DEFINES=

pushd ..\meta
cl %OPTS% ..\code\4ed_metagen.cpp /Femetagen
popd
pushd ..\code
"..\meta\metagen"
popd

pushd ..\build
call "..\code\buildsuper.bat" ..\code\4coder_default_bindings.cpp

set EXPORTS=/EXPORT:app_get_functions
cl %OPTS% %INCLUDES% %DEFINES% ..\code\4ed_app_target.cpp %* /Fe4ed_app /LD /link /INCREMENTAL:NO /OPT:REF %EXPORTS%

cl %OPTS% %INCLUDES% %DEFINES% ..\code\win32_4ed.cpp %LIBS% %ICON% %* /Fe4ed

popd



