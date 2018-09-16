@Echo Off
SetLocal EnableExtensions EnableDelayedExpansion
Title Pacman walk
Mode Con Cols=80 Lines=30
Bg Font 6
Color 07
Echo Move using the arrow keys.
Echo Press Esc for quit.
Bg Print E "Press any key to continue ... \01\n"
Pause > Nul
Cls
Rem Bg Font 2
PixelFnt 3
Bg Cursor 0


Set /a "row=0"
Set /a "col=0"
Set "spr_index=1"

Rem Play for 1 minute
Sound Play waka_waka.wav -1

Set "Last=333"
Set "STOP="
:Loop
For /L %%L In (1,1,300) do If Not Defined STOP (
Set /a "fps=0"
Set "prev_row=!row!"
Set "prev_col=!col!"
Bg LastKbd
Set "key=!ErrorLevel!"
If 27 Equ !key! Set "STOP=1"
Set /a "moved=0"
If 0 Equ !key! Set "key=!Last!"
If 328 Equ !key! (Set /a "row-=1,moved=1"
) Else If 336 Equ !key! (Set /a "row+=1,moved=1"
) Else If 331 Equ !key! (Set /a "col-=1,moved=1"
) Else If 333 Equ !key! (Set /a "col+=1,moved=1"
) Else Set "key=!Last!"
Set "Last=!key!"
If 1 Equ !moved! (
Sprite !prev_row! !prev_col! pac_erase.spr !row! !col! pac_!spr_index!.spr
) Else (
Sprite !row! !col! pac_!spr_index!.spr
)
Set /a "spr_index=(spr_index %% 2) + 1"
)
If Not Defined STOP Goto :Loop

:Bye
Sound Stop waka_waka.wav
Bg Font 6
Goto :Eof
