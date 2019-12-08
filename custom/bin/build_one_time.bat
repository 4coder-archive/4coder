@echo off

REM usage: <script> <target> [binary-output-path [mode]]
REM  src-file : a relative path to the target of a unity-build for a one time executable
REM  binary-output-path : a relative path where the generator.exe will be written, 
REM                       assumed to be "." if unset
REM  mode : if set to "release" builds with optimizations

set location=%cd%
set me="%~dp0"
cd %me%
cd ..
set custom_root=%cd%
set custom_bin=%custom_root%\bin
cd %location%

if NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (call "%custom_root%\windows_scripts\setup_cl_x64.bat")

set target=%1
if "%target%" == "" (echo error: no input file & exit)
set full_target=%target%
if NOT "%target:~1,1%" == ":" (set full_target="%cd%\%target%")

set dst=%2
if "%dst%" == "" (set dst=".")

set debug=/Zi
set release=/O2 /Zi
set mode=%debug%
if "%3" == "release" (set mode=%release%)

set opts=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4457 /wd4146 /WX
set opts=%opts% /GR- /nologo /FC
set opts=%opts% /I%custom_root%
set opts=%opts% %mode%

pushd %dst%
call cl /I"%custom_root%" %opts% %full_target% /Feone_time
popd



