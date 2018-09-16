_Based on https://web.archive.org/web/20180902083616/http://consolesoft.com/old-school/_

# Old School

These are basic programs for MS-DOS. You also can use it on FreeDOS.

## REPLY.COM

Get a keyboard input, and return the key number in errorlevel.

If the key is extended the second code is returned.

## MICE.COM

Get a click of mouse from the primary button, and print the row and the column. The index begin in `1`.

## WRICHR.COM

Print to screen one ansi byte. You can redirect it for save it.

Example:

```
wrichr.com 9 > 9.chr
```
