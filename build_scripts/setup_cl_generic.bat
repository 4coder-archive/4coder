@echo off

IF NOT DEFINED LIB (call "C:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" %1)
IF NOT DEFINED LIB (call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" %1)


