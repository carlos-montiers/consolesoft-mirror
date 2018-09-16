/*
  Copyright (C) 2010-2014 Carlos Montiers Aguilera

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Carlos Montiers Aguilera
  cmontiers@gmail.com
 */

/*
 memory-mapped-file.h
 header for memory-mapped-file.c
 version: 0.5
 */

#define UNICODE
#include <windows.h>
#include <stdlib.h>

struct MemoryMappedFile {
    const void *buffer;
    HANDLE hFileMappingObject;
    HANDLE hFile;
    LARGE_INTEGER li; //li.LowPart have the normal size
    UINT32 opened;
};

BOOL createMemoryMappedFile(LPCWSTR fileName, struct MemoryMappedFile *mmfile);
void closeMemoryMappedFile(struct MemoryMappedFile *mmfile);
