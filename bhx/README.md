# BHX 5.9 - Encode a binary file in a batch script for rebuild it.

## BHX 5.9
Encode a binary file in a batch script for rebuild it.

By default, the generated batch script name is mybin.cmd.
Is recommended convert your file in a cabinet
with compression, created with this command line:
Makecab /d compressiontype=lzx file file.cab
and use the generated cabinet as source.
This can save many bytes in the output script.

```
BHX source ... [/hex] [/o:out] [/y]

  source  Binary to encode.
  /hex    Encode binary data using 16 hexadecimal characters.
          The default encoder uses 85 printable characters.
  /o:out  out is the output filename instead of mybin.cmd.
  /y      Overwrite the output file.

BHX /author
  display the author of the program.
```

## Example of a generated script for rebuild self

`examples/build_bhx.cmd`

## Example of usage:

```
REM For save bytes is recommended create a cabinet file
REM using the compression LZX because it generally have
REM better compression than the default MSZIP

C:\folder> Makecab /D CompressionType=LZX file file.cab

REM Apply bhx

C:\folder> Bhx file.cab
"mybin.cmd" generated.

REM Check integrity:

C:\folder> Rename file file.bak

C:\folder> mybin.cmd

C:\folder> fc /b file file.bak
FC: no differences encountered
```

## Changelog:

```
v5.9 :
	+Fixed bugs that cause rebuild failed on windows 10.
	 (Thanks to Petr Lázňovský for report).
v5.8 :
	*Minor changes.
v5.7 :
	+Allow encode more than one file.
	*Updated the Rebuild function.
v5.6 :
	+Improved description of the program.
	+Bugfix.
v5.5 :
	*Minor changes.
v5.4 :
	+Add default encoding method using 85 characters,
	 based on z85 encode.
	+Add /hex option for encode using 16 hexadecimal
	 characters. This was the default in previous versions.
v5.3 :
	+Fixed bug happened on some windows 7.
	 (Thanks to Petr Lázňovský for report).
	+When you specify a cabinet file, bhx not preserve the
	 attributes of the files inside it. For example, if cabinet
	 file have inside a file with hidden attribute, it will
	 have none attributes when you rebuild it.
v4.0 :
	+Improved decode speedy.
v3.1 :
	+Fixed bug happened on windows xp using bulgarian language pack.
	 (Thanks to Vasil Arnaudov for report).
```

Download Executable + Source (zip)

```
bhx.exe info:

Size:		8192 bytes
SHA-256:	a25421d42d6d12ff983af4e84fb41816147021334697a1e2ecfd1aedbd675767
```
