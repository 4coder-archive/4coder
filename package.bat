@echo off

pushd W:\4ed
call "misc\build_all.bat" /O2
copy build\4ed.exe current_dist\4coder\*
copy build\4ed_app.dll current_dist\4coder\*
copy data\* current_dist\4coder\*
copy code\README.txt current_dist\4coder\*
copy code\TODO.txt current_dist\4coder\*
del current_dist\4coder\basic.cpp

call "misc\build_all.bat" /O2 /DFRED_SUPER
copy build\4ed.exe current_dist_super\4coder\*
copy build\4ed_app.dll current_dist_super\4coder\*
copy code\buildsuper.bat current_dist_super\4coder\*
copy data\* current_dist_super\4coder\*
del current_dist_super\4coder\basic.cpp
copy code\4coder_*.h current_dist_super\4coder\*
copy code\4coder_*.cpp current_dist_super\4coder\*
copy code\README.txt current_dist_super\4coder\*
copy code\TODO.txt current_dist_super\4coder\*
copy code\SUPERREADME.txt current_dist_super\4coder\*
copy current_dist\4coder\3rdparty\* current_dist_super\4coder\3rdparty\*
del current_dist_super\4coder\*.pdb
del current_dist_super\4coder\*.lib
del current_dist_super\4coder\*.obj
del current_dist_super\4coder\4coder_custom.dll

del current_dist_power\power\* /F /Q
copy code\power\* current_dist_power\power\*

popd