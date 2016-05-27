@echo off

pushd W:\4ed\meta
cl %OPTS% ..\code\readme_generator.c /Fereadmegen
popd

pushd W:\4ed\code

..\meta\readmegen

call "build_all.bat" /O2 /DFRED_KEEP_ASSERT
copy ..\build\4ed.exe ..\current_dist\4coder\*
copy ..\build\4ed.pdb ..\current_dist\4coder\*
copy ..\build\4ed_app.dll ..\current_dist\4coder\*
copy ..\build\4ed_app.pdb ..\current_dist\4coder\*
copy ..\data\* ..\current_dist\4coder\*
copy README.txt ..\current_dist\4coder\*
copy TODO.txt ..\current_dist\4coder\*
del ..\current_dist\SUPERREADME.txt
del ..\current_dist\4coder\basic.cpp
del ..\current_dist\4coder\.4coder_settings

call "build_all.bat" /O2 /DFRED_SUPER /DFRED_KEEP_ASSERT
copy ..\build\4ed.exe ..\current_dist_super\4coder\*
copy ..\build\4ed.pdb ..\current_dist_super\4coder\*
copy ..\build\4ed_app.dll ..\current_dist_super\4coder\*
copy ..\build\4ed_app.pdb ..\current_dist_super\4coder\*
copy buildsuper.bat ..\current_dist_super\4coder\*
copy ..\data\* ..\current_dist_super\4coder\*
del ..\current_dist_super\4coder\basic.cpp
copy 4coder_*.h ..\current_dist_super\4coder\*
copy 4coder_*.cpp ..\current_dist_super\4coder\*
copy README.txt ..\current_dist_super\4coder\*
copy TODO.txt ..\current_dist_super\4coder\*
copy SUPERREADME.txt ..\current_dist_super\4coder\*
copy ..\current_dist\4coder\3rdparty\* ..\current_dist_super\4coder\3rdparty\*
REM del ..\current_dist_super\4coder\*.pdb
del ..\current_dist_super\4coder\*.lib
del ..\current_dist_super\4coder\*.obj
del ..\current_dist_super\4coder\4coder_custom.dll
del ..\current_dist_super\4coder\.4coder_settings

del ..\current_dist_power\power\* /F /Q
copy power\* ..\current_dist_power\power\*

popd

