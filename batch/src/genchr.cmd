:: This code creates one single byte. Parameter: <int>0-255
:: Teamwork of carlos, penpen, aGerman, dbenham, bluesxman, sst
:: Tested under Win2000, XP, Win7, Win8, Win10
:: Fixed issue on environments with dbcs codepage

@echo off
setlocal enableextensions
set "USAGE=echo:Usage: Supply an integer 0-255& goto :EOF"
if "%~1" equ "" %USAGE%
set /a "val=%~1" 2>nul
if "%~1" neq "%val%" %USAGE%
if %~1 lss 0    %USAGE%
if %~1 gtr 255  %USAGE%

for /f "tokens=*" %%a in ('chcp') do for %%b in (%%a) do set "cp=%%~nb"
set "options=/d compress=off /d reserveperdatablocksize=26"
set "cmd=cmd /d /c"
mode con cp select=437 >nul
if %~1 neq 26  (type nul >%~1.tmp
makecab %options% /d reserveperfoldersize=%~1 %~1.tmp %~1.chr >nul
type %~1.chr | (
(for /l %%N in (1 1 38) do pause)>nul&findstr "^">%~1.tmp)
>nul %cmd% copy /y %~1.tmp /a %~1.chr /b
del %~1.tmp
) else (%cmd% copy /y nul + nul /a 26.chr /a >nul)
mode con cp select=%cp% >nul
