:: These commands were supplied by Jason Hood

Copy "%SystemRoot%\system32\cmd.exe"
bwpatchw.exe cmd.exe -f cmd-utf8-new.patch
Copy /Y cmd.exe "%SystemRoot%\system32\cmdutf8.exe"
Set "key=HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\cmd.exe"
Reg.exe add "%key%" /v "Debugger" /d "%SystemRoot%\system32\cmdutf8.exe" /f
