@echo off

IF "%3" == "" (echo need 3 parameters & GOTO END)

SET fake=%1
SET maj=%2
SET min=%3

SET vr=%fake%.%maj%.%min%
SET fv=%fake%-%maj%-%min%

SET flags=--fix-permissions --userversion=%vr%

pushd ..

SET dir=%CD%\distributions

butler push %flags% %dir%\demo_x64\4coder-%fv%-demo-win-x64.zip    4coder/4coder:win-x64-demo
butler push %flags% %dir%\super_x64\4coder-%fv%-super-win-x64.zip  4coder/4coder:win-x64

REM butler push %flags% %dir%\demo_x86\4coder-%fv%-demo-win-x86.zip    4coder/4coder:win-x86-demo
REM butler push %flags% %dir%\super_x86\4coder-%fv%-super-win-x86.zip  4coder/4coder:win-x86

popd

:END
