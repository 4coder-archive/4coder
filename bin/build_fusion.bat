@echo off

set code=%cd%
cd ..\\build
call %code%\\custom\\bin\\buildsuper_x64-win.bat %code%\\fusion\\4coder_fusion.cpp

cd %code%
