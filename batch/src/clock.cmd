@echo off
setlocal enableextensions enabledelayedexpansion

::Batch clock
:: consolesoft.com/batch/?#clock
::Code author: Carlos Montiers Aguilera
::Last updated: 2015, 19 february: minor changes
::Updated: 2014, 31 march: minor changes
::Updated: 2013, 18 november: reduced cpu usage fixed
::Created: 2013, october
::
::the clock automatically updates the display
::note: only open 1 instance of this script
::else it may fail
::for use 24 hour clock set /a "format=24"
::for use 12 hour clock set /a "format=12"
set /a "format=12"
::
mode con cols=23 lines=10
color 0a

Set "diga=ллл л ллллллл лллллллллллллллл"
Set "digb=л л л   л  лл лл  л    лл лл л"
Set "digc=л л л ллллллллллллллл  ллллллл"
Set "digd=л л л л    л  л  лл л  лл л  л"
Set "dige=ллл л лллллл  ллллллл  ллллллл"
set "m=JanFebMarAprMayJunJulAugSepOctNovDec"
set "w=MonTueWedThuFriSatSun"

call :getdate.init
call :getdate

:clock
set "th1=%day%%month%%year%%hour%%minute%"
set "da=."
set "db=."
if 12 equ !format! (set "db=M"
set "da=A"
if 12 leq %hour% (set /a "hour-=12"
set "da=P"
)
if 0 equ !hour! set "hour=12"
)

set /a "h1=hour/10,h2=hour-10*h1"
set /a "m1=minute/10,m2=minute-10*m1"

for %%_ in (h1 h2 m1 m2) do (set /a "index=3*%%_"
for %%# in (!index!) do (
set "%%_a=!diga:~%%#,3!"
set "%%_b=!digb:~%%#,3!"
set "%%_c=!digc:~%%#,3!"
set "%%_d=!digd:~%%#,3!"
set "%%_e=!dige:~%%#,3!"
))

set /a "index=3*(month-1)"
Set "mfp=!m:~%index%,3!"
set /a "index=3*(weekday-1)"
Set "wdp=!w:~%index%,3!"
set "dfp=%day%"
if %dfp% lss 10 set "dfp= %dfp%"

cls
if 12 equ !format! (title %h1%%h2%:%m1%%m2% %da%%db%
) else title %h1%%h2%:%m1%%m2%
echo(
echo    %h1a% %h2a%   %m1a% %m2a%
echo    %h1b% %h2b%   %m1b% %m2b%
echo    %h1c% %h2c% %da% %m1c% %m2c%
echo    %h1d% %h2d% %db% %m1d% %m2d%
echo    %h1e% %h2e%   %m1e% %m2e%
echo(
echo    ФФФФФФФТФФТФФФФФФ
echo      %wdp%  Г%dfp%Г %mfp%

:upd
call :getdate
set "th2=%day%%month%%year%%hour%%minute%"
if not "%th1%"=="%th2%" goto :clock
ping -l 0 -n 1 -w 500 1.0.0.0 >nul 2>&1
goto :upd

:getdate.init
set /a "jan=1,feb=2,mar=3,apr=4,may=5,jun=6,jul=7"
set /a "aug=8,sep=9,oct=10,nov=11,dec=12"
set /a "mon=1,tue=2,wed=3,thu=4,fri=5,sat=6,sun=7"
(echo .set infheader=""
echo .set infsectionorder=""
echo .set inffooter="%%2"
for /l %%# in (1,1,4) do echo .set inffooter%%#=""
echo .set cabinet="off"
echo .set compress="off"
echo .set donotcopyfiles="on"
echo .set rptfilename="nul"
) >"!temp!\~foo.ddf" 2>nul
set "random="
set "tf=!temp!\~%random%"
goto :eof

:getdate
makecab /d inffilename="!tf!" /f "!temp!\~foo.ddf" >nul 2>&1
for /f "usebackq tokens=1-7 delims=: " %%a in ("%tf%") do (
set /a "year=%%g,month=%%b,day=1%%c-100,weekday=%%a"
set /a "hour=1%%d-100,minute=1%%e-100,second=1%%f-100")
del "%tf%" >nul 2>&1
goto :eof
