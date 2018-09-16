/*
  Copyright (C) 2015 Carlos Montiers Aguilera

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
 * PIXELFNT 1.5
 * Set in the current user session to cmd.exe, until reboot, a 1x1, 2x2 and 3x3 Raster Font
 *
 * For compilation with tdm-gcc :
 * gcc -Wl,-e,__start -nostartfiles -m32 -O3 -s pixelfnt.c memory-mapped-file.c -lgdi32 -o PIXELFNT.EXE -fno-omit-frame-pointer -Wall
 *
 * Acknowledgments: Antonio Perez Ayala A.K.A Aacini for the idea of use a 1x1 font,
 * as expresed here: http://www.dostips.com/forum/viewtopic.php?p=31635
 */

#define UNICODE
#include <windows.h>
#include <stdio.h>
#include "memory-mapped-file.h"

typedef struct {
    int newmode;
} _startupinfo;

void __cdecl __wgetmainargs(int *pargc, wchar_t *** pargv,
        wchar_t *** penv, int globb, _startupinfo *);

void _start(void);
BOOL FileExistsW(wchar_t * szPath);
BOOL BufferIsEqualToContentOfFileW(const BYTE * buffer, DWORD buffer_size,
        LPCWSTR fileName);

// Undocumented functions, structures and messages
// BEGIN
BOOL WINAPI SetConsoleFont(HANDLE hConsoleOutput, DWORD nFont);
#define WM_SETCONSOLEINFO   (WM_USER+201)

typedef struct _CONSOLE_INFO {
    ULONG Length;
    COORD ScreenBufferSize;
    COORD WindowSize;
    ULONG WindowPosX;
    ULONG WindowPosY;
    COORD FontSize;
    ULONG FontFamily;
    ULONG FontWeight;
    WCHAR FaceName[32];
    ULONG CursorSize;
    ULONG FullScreen;
    ULONG QuickEdit;
    ULONG AutoPosition;
    ULONG InsertMode;
    USHORT ScreenColors;
    USHORT PopupColors;
    ULONG HistoryNoDup;
    ULONG HistoryBufferSize;
    ULONG NumberOfHistoryBuffers;
    COLORREF ColorTable[16];
    ULONG CodePage;
    HWND Hwnd;
    WCHAR ConsoleTitle[0x100];
    BYTE Padding[0x20];

} CONSOLE_INFO;
// END

//typedef struct _CONSOLE_FONT_INFOEX {
//    ULONG cbSize;
//    DWORD nFont;
//    COORD dwFontSize;
//    UINT FontFamily;
//    UINT FontWeight;
//    WCHAR FaceName[LF_FACESIZE];
//} CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;

//typedef struct _CONSOLE_SCREEN_BUFFER_INFOEX {
//    ULONG cbSize;
//    COORD dwSize;
//    COORD dwCursorPosition;
//    WORD wAttributes;
//    SMALL_RECT srWindow;
//    COORD dwMaximumWindowSize;
//    WORD wPopupAttributes;
//    BOOL bFullscreenSupported;
//    COLORREF ColorTable[16];
//} CONSOLE_SCREEN_BUFFER_INFOEX, *PCONSOLE_SCREEN_BUFFER_INFOEX;

//BOOL WINAPI SetCurrentConsoleFontEx(HANDLE, BOOL, PCONSOLE_FONT_INFOEX);
typedef BOOL(WINAPI * Func_SetCurrentConsoleFontEx) (HANDLE, BOOL,
        PCONSOLE_FONT_INFOEX);

//BOOL WINAPI GetConsoleScreenBufferInfoEx(HANDLE hConsoleOutput,
//                                       PCONSOLE_SCREEN_BUFFER_INFOEX
//                                       lpConsoleScreenBufferInfoEx);
typedef BOOL(WINAPI * Func_GetConsoleScreenBufferInfoEx) (HANDLE,
        PCONSOLE_SCREEN_BUFFER_INFOEX);

//BOOL WINAPI SetConsoleScreenBufferInfoEx(HANDLE hConsoleOutput,
//                                       PCONSOLE_SCREEN_BUFFER_INFOEX
//                                       lpConsoleScreenBufferInfoEx);
typedef BOOL(WINAPI * Func_SetConsoleScreenBufferInfoEx) (HANDLE,
        PCONSOLE_SCREEN_BUFFER_INFOEX);

BOOL SetConsoleInfo(HWND hwndConsole, CONSOLE_INFO * pci);

//int WINAPI AddFontResourceExW(LPCWSTR, DWORD, PVOID);
//HWND WINAPI GetConsoleWindow(void);

const COLORREF DefaultColorTable[16] = {
    RGB(0, 0, 0),
    RGB(0, 0, 128),
    RGB(0, 128, 0),
    RGB(0, 128, 128),
    RGB(128, 0, 0),
    RGB(128, 0, 128),
    RGB(128, 128, 0),
    RGB(192, 192, 192),
    RGB(128, 128, 128),
    RGB(0, 0, 255),
    RGB(0, 255, 0),
    RGB(0, 255, 255),
    RGB(255, 0, 0),
    RGB(255, 0, 255),
    RGB(255, 255, 0),
    RGB(255, 255, 255)
};

const BYTE PIXELFNT1[292] = {
    0x00, 0x02, 0x24, 0x01, 0x00, 0x00, 0x43, 0x61, 0x72, 0x6C, 0x6F,
    0x73, 0x20, 0x4D, 0x6F, 0x6E, 0x74, 0x69, 0x65, 0x72, 0x73, 0x20,
    0x41, 0x67, 0x75, 0x69, 0x6C, 0x65, 0x72, 0x61, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x64, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0xFF, 0x01, 0x00,
    0x01, 0x00, 0x30, 0x01, 0x00, 0x01, 0x00, 0x01, 0x20, 0x01, 0x01,
    0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFA, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0xFA,
    0x00, 0x01, 0x00, 0xFB, 0x00, 0x01, 0x00, 0xFC, 0x00, 0x01, 0x00,
    0xFD, 0x00, 0x01, 0x00, 0xFE, 0x00, 0x01, 0x00, 0xFF, 0x00, 0x01,
    0x00, 0x00, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x00, 0x02, 0x01,
    0x01, 0x00, 0x03, 0x01, 0x01, 0x00, 0x04, 0x01, 0x01, 0x00, 0x05,
    0x01, 0x01, 0x00, 0x06, 0x01, 0x01, 0x00, 0x07, 0x01, 0x01, 0x00,
    0x08, 0x01, 0x01, 0x00, 0x09, 0x01, 0x01, 0x00, 0x0A, 0x01, 0x01,
    0x00, 0x0B, 0x01, 0x01, 0x00, 0x0C, 0x01, 0x01, 0x00, 0x0D, 0x01,
    0x01, 0x00, 0x0E, 0x01, 0x01, 0x00, 0x0F, 0x01, 0x01, 0x00, 0x10,
    0x01, 0x01, 0x00, 0x11, 0x01, 0x01, 0x00, 0x12, 0x01, 0x01, 0x00,
    0x13, 0x01, 0x01, 0x00, 0x14, 0x01, 0x01, 0x00, 0x15, 0x01, 0x01,
    0x00, 0x16, 0x01, 0x01, 0x00, 0x17, 0x01, 0x01, 0x00, 0x18, 0x01,
    0x01, 0x00, 0x19, 0x01, 0x08, 0x00, 0x1A, 0x01, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x54, 0x65, 0x72,
    0x6D, 0x69, 0x6E, 0x61, 0x6C, 0x00
};

const BYTE PIXELFNT2[325] = {
    0x00, 0x02, 0x45, 0x01, 0x00, 0x00, 0x43, 0x61, 0x72, 0x6C, 0x6F,
    0x73, 0x20, 0x4D, 0x6F, 0x6E, 0x74, 0x69, 0x65, 0x72, 0x73, 0x20,
    0x41, 0x67, 0x75, 0x69, 0x6C, 0x65, 0x72, 0x61, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x64, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0xFF, 0x02, 0x00,
    0x02, 0x00, 0x30, 0x02, 0x00, 0x02, 0x00, 0x01, 0x20, 0x01, 0x01,
    0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFA, 0x00, 0x00, 0x00, 0xC4, 0x02, 0x00, 0xFA,
    0x00, 0x02, 0x00, 0xFC, 0x00, 0x02, 0x00, 0xFE, 0x00, 0x02, 0x00,
    0x00, 0x01, 0x02, 0x00, 0x02, 0x01, 0x02, 0x00, 0x04, 0x01, 0x02,
    0x00, 0x06, 0x01, 0x02, 0x00, 0x08, 0x01, 0x02, 0x00, 0x0A, 0x01,
    0x02, 0x00, 0x0C, 0x01, 0x02, 0x00, 0x0E, 0x01, 0x02, 0x00, 0x10,
    0x01, 0x02, 0x00, 0x12, 0x01, 0x02, 0x00, 0x14, 0x01, 0x02, 0x00,
    0x16, 0x01, 0x02, 0x00, 0x18, 0x01, 0x02, 0x00, 0x1A, 0x01, 0x02,
    0x00, 0x1C, 0x01, 0x02, 0x00, 0x1E, 0x01, 0x02, 0x00, 0x20, 0x01,
    0x02, 0x00, 0x22, 0x01, 0x02, 0x00, 0x24, 0x01, 0x02, 0x00, 0x26,
    0x01, 0x02, 0x00, 0x28, 0x01, 0x02, 0x00, 0x2A, 0x01, 0x02, 0x00,
    0x2C, 0x01, 0x02, 0x00, 0x2E, 0x01, 0x02, 0x00, 0x30, 0x01, 0x02,
    0x00, 0x32, 0x01, 0x02, 0x00, 0x34, 0x01, 0x02, 0x00, 0x36, 0x01,
    0x02, 0x00, 0x38, 0x01, 0x08, 0x00, 0x3A, 0x01, 0xC0, 0xC0, 0xC0,
    0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xC0, 0xC0, 0x00,
    0x00, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
    0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
    0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0, 0xC0,
    0xC0, 0xC0, 0xC0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x54, 0x65, 0x72,
    0x6D, 0x69, 0x6E, 0x61, 0x6C, 0x00
};

const BYTE PIXELFNT3[358] = {
    0x00, 0x02, 0x66, 0x01, 0x00, 0x00, 0x43, 0x61, 0x72, 0x6C, 0x6F,
    0x73, 0x20, 0x4D, 0x6F, 0x6E, 0x74, 0x69, 0x65, 0x72, 0x73, 0x20,
    0x41, 0x67, 0x75, 0x69, 0x6C, 0x65, 0x72, 0x61, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x64, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0xFF, 0x03, 0x00,
    0x03, 0x00, 0x30, 0x03, 0x00, 0x03, 0x00, 0x01, 0x20, 0x01, 0x01,
    0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5D, 0x01, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0xFA, 0x00, 0x00, 0x00, 0xC4, 0x03, 0x00, 0xFA,
    0x00, 0x03, 0x00, 0xFD, 0x00, 0x03, 0x00, 0x00, 0x01, 0x03, 0x00,
    0x03, 0x01, 0x03, 0x00, 0x06, 0x01, 0x03, 0x00, 0x09, 0x01, 0x03,
    0x00, 0x0C, 0x01, 0x03, 0x00, 0x0F, 0x01, 0x03, 0x00, 0x12, 0x01,
    0x03, 0x00, 0x15, 0x01, 0x03, 0x00, 0x18, 0x01, 0x03, 0x00, 0x1B,
    0x01, 0x03, 0x00, 0x1E, 0x01, 0x03, 0x00, 0x21, 0x01, 0x03, 0x00,
    0x24, 0x01, 0x03, 0x00, 0x27, 0x01, 0x03, 0x00, 0x2A, 0x01, 0x03,
    0x00, 0x2D, 0x01, 0x03, 0x00, 0x30, 0x01, 0x03, 0x00, 0x33, 0x01,
    0x03, 0x00, 0x36, 0x01, 0x03, 0x00, 0x39, 0x01, 0x03, 0x00, 0x3C,
    0x01, 0x03, 0x00, 0x3F, 0x01, 0x03, 0x00, 0x42, 0x01, 0x03, 0x00,
    0x45, 0x01, 0x03, 0x00, 0x48, 0x01, 0x03, 0x00, 0x4B, 0x01, 0x03,
    0x00, 0x4E, 0x01, 0x03, 0x00, 0x51, 0x01, 0x03, 0x00, 0x54, 0x01,
    0x03, 0x00, 0x57, 0x01, 0x08, 0x00, 0x5A, 0x01, 0xE0, 0xE0, 0xE0,
    0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
    0xE0, 0xE0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
    0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
    0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
    0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
    0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
    0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0, 0xE0,
    0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x65, 0x72,
    0x6D, 0x69, 0x6E, 0x61, 0x6C, 0x00
};

typedef struct _FNTFILE {
    WCHAR *filename;
    const BYTE *buffer;
    size_t size_buffer;
} FNTFILE;

void _start(void) {
    INT result = 1;

    int argc;
    wchar_t **argv;
    wchar_t **env;
    _startupinfo start_info = {0};

    HANDLE hOut = INVALID_HANDLE_VALUE;
    HANDLE hIn = INVALID_HANDLE_VALUE;
    HINSTANCE dllHandle;
    Func_SetCurrentConsoleFontEx SetCurrentConsoleFontEx_Ptr;
    Func_SetConsoleScreenBufferInfoEx SetConsoleScreenBufferInfoEx_Ptr;
    Func_GetConsoleScreenBufferInfoEx GetConsoleScreenBufferInfoEx_Ptr;

    HANDLE hFile;
    DWORD bytesWrited;

#define LENGHT_CHARACTERS_TO_COPY (16)
#define LENGHT_FNT_FILENAME (682)
#define LENGTH_TEMP_DIR (LENGHT_FNT_FILENAME - LENGHT_CHARACTERS_TO_COPY)
    WCHAR TEMP_DIR[LENGTH_TEMP_DIR];
    WCHAR FNT_FILENAME_1[LENGHT_FNT_FILENAME];
    WCHAR FNT_FILENAME_2[LENGHT_FNT_FILENAME];
    WCHAR FNT_FILENAME_3[LENGHT_FNT_FILENAME];
#define FONTS_SIZE 3
    FNTFILE FONTS[FONTS_SIZE] = {
        {FNT_FILENAME_1, PIXELFNT1, sizeof (PIXELFNT1)}
        ,
        {FNT_FILENAME_2, PIXELFNT2, sizeof (PIXELFNT2)}
        ,
        {FNT_FILENAME_3, PIXELFNT3, sizeof (PIXELFNT3)}
    };
    FNTFILE *fnt;
    WCHAR *name_template = L"%ls\\TEMP\\PIXEL%d.FNT";

    BOOL generated;
    INT copied;
    INT needed;
    CONSOLE_INFO console_info;
    CONSOLE_CURSOR_INFO cursor_info;
    CONSOLE_FONT_INFOEX font_infoex;
    CONSOLE_SCREEN_BUFFER_INFO screen_buffer_info;
    CONSOLE_SCREEN_BUFFER_INFOEX screen_buffer_infoex;
    SMALL_RECT windowSize;
    DWORD consoleMode;
    HWND cmdself;
    UINT codePage;
    INT font_number;
    INT i;
    INT corrupted_fnts;

    argv = NULL;
    __wgetmainargs(&argc, &argv, &env, 0, &start_info);
    if (NULL == argv) {
        ExitProcess(-1);
    }

    if (2 != argc) {
        wprintf(L"PIXELFNT v1.5\n"
                "Programmed by Carlos Montiers Aguilera\n\n"
                "Usage PIXELFNT 1|2|3\n"
                " where:\n" " 1 = 1x1\n" " 2 = 2x2\n" " 3 = 3x3\n\n");
        ExitProcess(0);
    }

    font_number = wcstol(argv[1], (wchar_t **) NULL, 10);

    if ((font_number < 1) || (font_number > 3)) {
        wprintf(L"Incorrect parameter.\n");
        goto bye;
    }


    generated = FALSE;
    needed = GetWindowsDirectoryW(NULL, 0);
    if ((needed >= 4)
            && ((needed - 1) < LENGTH_TEMP_DIR)
            ) {
        copied = GetWindowsDirectoryW(TEMP_DIR, LENGTH_TEMP_DIR);

        if (needed == (copied + 1)) {
            TEMP_DIR[copied] = L'\0';
            if (L'\\' == TEMP_DIR[copied - 1]) {
                TEMP_DIR[copied - 1] = L'\0';
            }

            _swprintf(FNT_FILENAME_1, name_template, TEMP_DIR, 1);
            _swprintf(FNT_FILENAME_2, name_template, TEMP_DIR, 2);
            _swprintf(FNT_FILENAME_3, name_template, TEMP_DIR, 3);
            generated = TRUE;
        }

    }

    if (!generated) {
        wprintf(L"Generate the filenames was not possible\n");
        goto bye;
    }

    corrupted_fnts = FONTS_SIZE;
    for (i = 0; i < FONTS_SIZE; i++) {
        fnt = &FONTS[i];

        if (!FileExistsW(fnt->filename)) {

            hFile = CreateFileW(fnt->filename,
                    GENERIC_WRITE,
                    0,
                    NULL, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY,
                    NULL);

            if (INVALID_HANDLE_VALUE != hFile) {
                BOOL success;
                success = WriteFile(hFile,
                        fnt->buffer,
                        fnt->size_buffer, &bytesWrited, NULL);
                CloseHandle(hFile);
                if (!success) {
                    DeleteFileW(fnt->filename);
                }
            }
        }

        if (!BufferIsEqualToContentOfFileW
                (fnt->buffer, fnt->size_buffer, fnt->filename)) {
            wprintf(L"%ls is damaged\n", fnt->filename);
        } else {
            --corrupted_fnts;
        }

    }

    if (corrupted_fnts) {
        goto bye;
    }

    hOut = CreateFileW(L"CONOUT$",
            (GENERIC_READ | GENERIC_WRITE),
            (FILE_SHARE_READ | FILE_SHARE_WRITE),
            NULL, OPEN_EXISTING, 0, NULL);

    hIn = CreateFileW(L"CONIN$",
            (GENERIC_READ | GENERIC_WRITE),
            (FILE_SHARE_READ | FILE_SHARE_WRITE),
            NULL, OPEN_EXISTING, 0, NULL);

    if ((INVALID_HANDLE_VALUE == hOut) || (INVALID_HANDLE_VALUE == hIn)) {
        wprintf(L"Invalid output handle\n");
        goto bye;
    }

    for (i = 0; i < FONTS_SIZE; i++) {
        AddFontResourceExW(FONTS[i].filename, 0, 0);
    }

    dllHandle = LoadLibraryW(L"KERNEL32.DLL");

    if (!dllHandle) {
        wprintf(L"Error getting handle");
        goto bye;
    }


    SetCurrentConsoleFontEx_Ptr =
            (Func_SetCurrentConsoleFontEx) GetProcAddress(dllHandle,
            "SetCurrentConsoleFontEx");

    SetConsoleScreenBufferInfoEx_Ptr =
            (Func_SetConsoleScreenBufferInfoEx) GetProcAddress(dllHandle,
            "SetConsoleScreenBufferInfoEx");

    GetConsoleScreenBufferInfoEx_Ptr =
            (Func_GetConsoleScreenBufferInfoEx) GetProcAddress(dllHandle,
            "GetConsoleScreenBufferInfoEx");

    if (SetCurrentConsoleFontEx_Ptr) //is vista
    {
        font_infoex.cbSize = sizeof (font_infoex);
        font_infoex.nFont = (font_number - 1);
        font_infoex.dwFontSize.X = font_number;
        font_infoex.dwFontSize.Y = font_number;
        font_infoex.FontFamily = 48;
        font_infoex.FontWeight = 400;
        wcscpy(font_infoex.FaceName, L"Terminal");

        SetCurrentConsoleFontEx_Ptr(hOut, FALSE, &font_infoex);
        SetConsoleFont(hOut, (font_number - 1));

        if (GetConsoleScreenBufferInfoEx_Ptr
                && SetConsoleScreenBufferInfoEx_Ptr) {

            screen_buffer_infoex.cbSize = sizeof (screen_buffer_infoex);
            GetConsoleScreenBufferInfoEx_Ptr(hOut, &screen_buffer_infoex);

            screen_buffer_infoex.wPopupAttributes = MAKEWORD(0x5, 0xf);
            screen_buffer_infoex.bFullscreenSupported = FALSE;

            memcpy(screen_buffer_infoex.ColorTable, DefaultColorTable,
                    sizeof (screen_buffer_infoex.ColorTable));

            SetConsoleScreenBufferInfoEx_Ptr(hOut, &screen_buffer_infoex);

        }

    } else //is xp
    {

#ifndef ENABLE_QUICK_EDIT_MODE
#define ENABLE_QUICK_EDIT_MODE (0x0040)
#endif
#ifndef ENABLE_INSERT_MODE
#define ENABLE_INSERT_MODE (0x0020)
#endif

        cmdself = GetConsoleWindow();
        codePage = GetConsoleOutputCP();

        GetConsoleScreenBufferInfo(hOut, &screen_buffer_info);
        GetConsoleMode(hIn, &consoleMode);
        GetConsoleCursorInfo(hOut, &cursor_info);

        memset(&console_info, 0, sizeof (console_info));
        console_info.Length = sizeof (console_info);
        console_info.ScreenBufferSize = screen_buffer_info.dwSize;
        console_info.WindowSize.X =
                screen_buffer_info.srWindow.Right -
                screen_buffer_info.srWindow.Left + 1;
        console_info.WindowSize.Y =
                screen_buffer_info.srWindow.Bottom -
                screen_buffer_info.srWindow.Top + 1;
        console_info.WindowPosX = screen_buffer_info.srWindow.Left;
        console_info.WindowPosY = screen_buffer_info.srWindow.Top;

        console_info.FontSize.X = font_number;
        console_info.FontSize.Y = font_number;
        console_info.FontFamily = 48;
        console_info.FontWeight = 400;

        wcscpy(console_info.FaceName, L"Terminal");

        console_info.CursorSize = cursor_info.dwSize;
        console_info.FullScreen = FALSE;
        console_info.QuickEdit =
                ((ENABLE_QUICK_EDIT_MODE) ==
                (consoleMode & ENABLE_QUICK_EDIT_MODE));
        console_info.AutoPosition = 0x10000;
        console_info.InsertMode =
                ((ENABLE_INSERT_MODE) == (consoleMode & ENABLE_INSERT_MODE));
        console_info.ScreenColors = screen_buffer_info.wAttributes;
        console_info.PopupColors = MAKEWORD(0x5, 0xf);
        console_info.HistoryNoDup = FALSE;
        console_info.HistoryBufferSize = 50;
        console_info.NumberOfHistoryBuffers = 4;

        memcpy(console_info.ColorTable, DefaultColorTable,
                sizeof (DefaultColorTable));

        console_info.CodePage = codePage;
        console_info.Hwnd = cmdself;

        SetConsoleInfo(cmdself, &console_info);
        SetConsoleFont(hOut, (font_number - 1));

    }

    //update size of windows
    GetConsoleScreenBufferInfo(hOut, &screen_buffer_info);
    windowSize.Left = (SHORT) 0;
    windowSize.Top = (SHORT) 0;
    windowSize.Right = (SHORT) screen_buffer_info.dwSize.X - 1;
    windowSize.Bottom = (SHORT) screen_buffer_info.dwSize.Y - 1;
    SetConsoleWindowInfo(hOut, TRUE, &windowSize);

    FreeLibrary(dllHandle);
    result = 0;

bye:
    if (INVALID_HANDLE_VALUE != hOut) {
        CloseHandle(hOut);
    }
    if (INVALID_HANDLE_VALUE != hIn) {
        CloseHandle(hIn);
    }
    ExitProcess(result);

}

BOOL FileExistsW(wchar_t * szPath) {
    BOOL result;
    DWORD dwAttrib;

    dwAttrib = GetFileAttributesW(szPath);

    result = (dwAttrib != INVALID_FILE_ATTRIBUTES)
            && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);

    return result;
}

BOOL BufferIsEqualToContentOfFileW(const BYTE * buffer, DWORD buffer_size,
        LPCWSTR fileName) {
    BOOL result = FALSE;
    struct MemoryMappedFile mmfile;
    DWORD i;
    BYTE *fBufferPtr;


    if (!createMemoryMappedFile(fileName, &mmfile)) {
        goto bufferisequaltocontentoffilew_bye;
    }

    if (!((mmfile.li.HighPart == 0) && (mmfile.li.LowPart == buffer_size))
            ) {
        goto bufferisequaltocontentoffilew_bye;
    }

    fBufferPtr = (BYTE *) mmfile.buffer;

    for (i = 0; i < mmfile.li.LowPart; i++) {
        if (*fBufferPtr != *buffer) {
            goto bufferisequaltocontentoffilew_bye;
        }
        fBufferPtr++;
        buffer++;
    }

    result = TRUE;

bufferisequaltocontentoffilew_bye:
    closeMemoryMappedFile(&mmfile);

    return result;
}

BOOL SetConsoleInfo(HWND hwndConsole, CONSOLE_INFO * pci) {
    DWORD dwConsoleOwnerPid;
    HANDLE hProcess;
    HANDLE hSection, hDupSection;
    PVOID ptrView = 0;

    // Open the process which "owns" the console
    GetWindowThreadProcessId(hwndConsole, &dwConsoleOwnerPid);
    hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwConsoleOwnerPid);

    // Create a SECTION object backed by page-file, then map a view of
    // this section into the owner process so we can write the contents
    // of the CONSOLE_INFO buffer into it
    hSection =
            CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0,
            pci->Length, 0);

    // Copy our console structure into the section-object
    ptrView =
            MapViewOfFile(hSection, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0,
            pci->Length);

    memcpy(ptrView, pci, pci->Length);

    UnmapViewOfFile(ptrView);

    // Map the memory into owner process
    DuplicateHandle(GetCurrentProcess(), hSection, hProcess, &hDupSection,
            0, FALSE, DUPLICATE_SAME_ACCESS);

    //  Send console window the "update" message
    SendMessage(hwndConsole, WM_SETCONSOLEINFO, (WPARAM) hDupSection, 0);

    CloseHandle(hSection);
    CloseHandle(hProcess);

    return TRUE;
}
