@echo off

REM "build_exp.bat" /O2
REM "build_all.bat" /DFRED_SUPER /DFRED_INTERNAL /Zi
REM "build_all.bat" /DFRED_INTERNAL /Zi
REM "build_all.bat" /DFRED_SUPER /O2 /Zi

call "ctime" -begin 4ed_data.ctm

SET OPTS=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4390 /WX
SET OPTS=/GR- /EHa- /nologo /FC

SET FirstError=0

pushd ..\build
cl %OPTS% ..\code\build.c /Febuild /DDEV_BUILD
if %ERRORLEVEL% neq 0 (set FirstError=1)
popd

..\build\build
if %ERRORLEVEL% neq 0 (set FirstError=1)

pushd ..\build
call "print_size.bat" 4coder_custom.dll
call "print_size.bat" 4ed_app.dll
call "print_size.bat" 4ed.exe
popd

call "ctime" -end 4ed_data.ctm %FirstError%