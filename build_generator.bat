@echo off

REM usage: build_generator <src-file> <binary-output-path> [release]
REM src-file: a relative path to the target of a unity-build for a generator
REM binary-output-path: a relative path where the generator.exe will be written, assumed to be "." if unset

set code_home=%~dp0
if %code_home:~-1%==\ (set code_home=%code_home:~0,-1%)

if NOT "%Platform%" == "X64" IF NOT "%Platform%" == "x64" (call "%code_home%\windows_scripts\setup_cl_x64.bat")

set src=%1
if "%src%" == "" (echo error: no input file & exit)

set dst=%2
if "%dst%" == "" (set dst=".")

set opts=/W4 /wd4310 /wd4100 /wd4201 /wd4505 /wd4996 /wd4127 /wd4510 /wd4512 /wd4610 /wd4457 /wd4146 /WX
set opts=%opts% /GR- /nologo /FC
set debug=/Zi
set release=/O2 /Zi

set mode=%debug%
if "%3" == "release" (set mode=%release%)

set full_src="%cd%\%src%"

pushd %dst%
call cl /I"%code_home%" %opts% %mode% %full_src% /Fegenerator
popd



