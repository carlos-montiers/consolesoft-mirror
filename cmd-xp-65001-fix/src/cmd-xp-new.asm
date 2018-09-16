; Patch XP's CMD.EXE (5.1.2600.5512) to work with UTF-8 batch files.
; Method discovered by Carlos, patch by adoxa.
; Created 14 may 2014
; Fixed 19 may 2014 by Carlos

<4ad06a57>
call 4ad20620
nop
<4ad0b077>
call 4ad20620
nop
<4ad16ee6>
call 4ad20620
nop
<4ad1afa5>
call 4ad20620
nop
<4ad1cedd>
call 4ad20620
nop

<4ad20620>
mov eax,[esp+4] ;; code page
test eax,eax
jnz short @f
push 4ad0497c ;; push lpModuleName =  L"kernel32.dll"
call dword[4ad0113c] ;; hModule = call GetModuleHandleW
push @GetACP ;; push lpProcName = "GetACP"
push eax ;; push hModule
call dword[4ad01138] ;; *func = GetProcAddress
call eax ;; call func()
@@:
cmp eax,50229.
je short @f
ja @bigger
cmp eax,42.
je short @f
cmp eax,50220.
jb short @ok
cmp eax,50222.
jbe short @f
cmp eax,50225.
je short @f
cmp eax,50227.
je short @f
@ok:
jmp dword[4ad01158] ;; MultiByteToWideChar
@bigger:
cmp eax,52936.
je short @f
cmp eax,54936.
je short @f
cmp eax,57002.
jb short @ok
cmp eax,57011.
jbe short @f
cmp eax,65000.
jb short @ok
cmp eax,65001.
ja short @ok
@@:
mov byte[esp+8],0 ;; flags
jmp dword[4ad01158] ;; MultiByteToWideChar

@GetACP@4: "GetACP\0"
;;end
