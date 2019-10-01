@echo off

REM Usage: zip <archivename>
REM compresses the current directory into a zip named <archivename>.zip

SET ZIP_PATH=C:\Program Files (x86)\7-Zip
IF NOT EXIST "%ZIP_PATH%" (SET ZIP_PATH=C:\Program Files\7-Zip)
IF EXIST "%ZIP_PATH%" ("%ZIP_PATH%\7z.exe" a -tzip -y %*)

