@echo off

REM Usage: zip <archivename>
REM compresses the current directory into a zip named <archivename>.zip

"C:\Program Files (x86)\7-Zip\7z.exe" a -tzip -y %*