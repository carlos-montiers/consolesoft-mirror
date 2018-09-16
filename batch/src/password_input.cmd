@Echo Off
SetLocal EnableExtensions EnableDelayedExpansion

Set /P "=Enter a Password:" < Nul
Call :PasswordInput
Echo(Your password text:!pwd!
Pause

Goto :Eof

:PasswordInput
:: consolesoft.com/batch/?#password_input
::Author: Carlos Montiers Aguilera
::Last updated: 20150405. Created: 20150401.
::Set variable pwd with input password
::Needs delayed expansion enabled
Set "pwd="
Set "INTRO=" &For /F "skip=1" %%# in (
'"Echo(|Replace.exe ? . /U /W"'
) Do If Not Defined INTRO Set "INTRO=%%#"
For /F %%# In (
'"Prompt $H &For %%_ In (_) Do Rem"') Do Set "BKSPACE=%%#"
:_PasswordInput_Kbd
Set "Excl="
Set "CHR=" &For /F skip^=1^ delims^=^ eol^= %%# in (
'Replace.exe ? . /U /W') Do If Not Defined CHR (
Set "CHR=%%#" &If "%%#"=="!" Set "Excl=yes")
If "!INTRO!"=="!CHR!" Echo(&Goto :Eof
If "!BKSPACE!"=="!CHR!" (If Defined pwd (
Set "pwd=!pwd:~0,-1!"
Set /P "=!BKSPACE! !BKSPACE!" <Nul)
Goto :_PasswordInput_Kbd
) Else If Not Defined Excl (
Set "pwd=!pwd!!CHR:~0,1!"
) Else Set "pwd=!pwd!^!"
Set /P "=*" <Nul
Goto :_PasswordInput_Kbd
