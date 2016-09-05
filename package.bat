REM @echo off
REM pushd W:\4ed\meta
REM cl %OPTS% ..\code\readme_generator.c /Fereadmegen
REM popd

REM pushd W:\4ed\code

REM ..\meta\readmegen

REM call "build_all.bat" /O2 /DFRED_KEEP_ASSERT /Zi
REM del ..\current_dist\4coder\*.html
REM copy ..\build\4ed.exe ..\current_dist\4coder\*
REM copy ..\build\4ed.pdb ..\current_dist\4coder\*
REM copy ..\build\4ed_app.dll ..\current_dist\4coder\*
REM copy ..\build\4ed_app.pdb ..\current_dist\4coder\*
REM copy ..\data\* ..\current_dist\4coder\*
REM copy README.txt ..\current_dist\4coder\*
REM copy TODO.txt ..\current_dist\4coder\*
REM del ..\current_dist\4coder\.4coder_settings

REM call "build_all.bat" /O2 /DFRED_SUPER /DFRED_KEEP_ASSERT /Zi
REM del ..\current_dist\4coder\*.html
REM copy ..\build\4ed.exe ..\current_dist_super\4coder\*
REM copy ..\build\4ed.pdb ..\current_dist_super\4coder\*
REM copy ..\build\4ed_app.dll ..\current_dist_super\4coder\*
REM copy ..\build\4ed_app.pdb ..\current_dist_super\4coder\*
REM copy buildsuper.bat ..\current_dist_super\4coder\*
REM copy ..\data\* ..\current_dist_super\4coder\*
REM del ..\current_dist_super\4coder\basic.cpp
REM copy 4coder_*.h ..\current_dist_super\4coder\*
REM copy 4coder_*.cpp ..\current_dist_super\4coder\*
REM copy README.txt ..\current_dist_super\4coder\*
REM copy TODO.txt ..\current_dist_super\4coder\*
REM copy ..\current_dist\4coder\3rdparty\* ..\current_dist_super\4coder\3rdparty\*
REM del ..\current_dist_super\4coder\*.lib
REM del ..\current_dist_super\4coder\*.obj
REM del ..\current_dist_super\4coder\4coder_custom.dll
REM del ..\current_dist_super\4coder\.4coder_settings

REM copy 4coder_API.html ..\current_dist_super\*

REM del ..\current_dist_power\power\* /F /Q
REM copy power\* ..\current_dist_power\power\*

REM popd

build.bat /DPACKAGE
