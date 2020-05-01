@echo off

call bin\build.bat /DPACKAGE_DEMO_X64
call bin\build.bat /DPACKAGE_DEMO_X86
call bin\build.bat /DPACKAGE_SUPER_X64
call bin\build.bat /DPACKAGE_SUPER_X86
