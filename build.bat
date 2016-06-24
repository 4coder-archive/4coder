@echo off


pushd ..\meta
REM cl %OPTS% ..\code\4ed_metagen.cpp /Zi /Femetagen
popd

pushd ..\code
REM "..\meta\metagen"
popd

REM "build_exp.bat" /Zi
"build_all.bat" /DFRED_SUPER /DFRED_INTERNAL /Zi
REM "build_all.bat" /O2 /Zi
