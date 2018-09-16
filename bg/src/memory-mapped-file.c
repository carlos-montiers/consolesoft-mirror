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
 memory-mapped-file.c
 structures for read a file as if in a memory buffer
 version: 0.5
 */

#include "memory-mapped-file.h"

// return TRUE success, FALSE error

BOOL createMemoryMappedFile(LPCWSTR fileName, struct MemoryMappedFile *mmfile) {

    mmfile->hFile = CreateFileW(fileName,
            GENERIC_READ,
            FILE_SHARE_READ, NULL, OPEN_EXISTING, 0,
            NULL);

    if (INVALID_HANDLE_VALUE == mmfile->hFile)
        goto mmfile_error_1;

    if (!GetFileSizeEx(mmfile->hFile, &mmfile->li))
        goto mmfile_error_2;

    mmfile->hFileMappingObject = CreateFileMappingW(mmfile->hFile,
            NULL,
            PAGE_READONLY,
            mmfile->li.HighPart,
            mmfile->li.LowPart,
            NULL);

    if (!mmfile->hFileMappingObject)
        goto mmfile_error_2;

    mmfile->buffer = MapViewOfFile(mmfile->hFileMappingObject,
            FILE_MAP_READ, 0, 0, 0);

    if (!mmfile->buffer)
        goto mmfile_error_3;

    mmfile->opened = TRUE;
    return TRUE;

mmfile_error_3:
    CloseHandle(mmfile->hFileMappingObject);
mmfile_error_2:
    CloseHandle(mmfile->hFile);
mmfile_error_1:

    mmfile->opened = FALSE;
    return FALSE;
}

void closeMemoryMappedFile(struct MemoryMappedFile *mmfile) {
    if (mmfile->opened) {
        UnmapViewOfFile(mmfile->buffer);
        CloseHandle(mmfile->hFileMappingObject);
        CloseHandle(mmfile->hFile);

        mmfile->opened = FALSE;
    }
}
