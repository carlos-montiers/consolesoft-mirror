/*
  Copyright (C) 2014 Carlos Montiers Aguilera
  Copyright (C) 2014 Romain GARBI

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
 * BG Sprite Editor Version 1.0.1
 * For compilation with tdm-gcc :
 * gcc -Wl,-e,__start -nostartfiles -m32 -Os editor.c -o EDITOR.EXE
 */

#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "memory-mapped-file.h"

#define _CONSOLE_APP 1

#define  MAX_SPRITE_LENGTH  (0xFE01) // = (0xFF * 0xFF)
static INT paintZone_Width = 0xFF;
static INT paintZone_Height = 0xFF;
static INT paintZone_Length = MAX_SPRITE_LENGTH;
static CHAR_INFO paintZone[MAX_SPRITE_LENGTH];

int editor_main(int argc, wchar_t ** argv);

typedef struct {
    int newmode;
} _startupinfo;

void __cdecl __set_app_type(int apptype);
int __wgetmainargs(int *_Argc, wchar_t *** _Argv, wchar_t *** _Env,
        int _DoWildCard, _startupinfo * _StartInfo);

void _start(void) {
    _startupinfo start_info = {0};
    int argc;
    WCHAR **argv;
    WCHAR **env;

    /* Sets the current application type */
    __set_app_type(_CONSOLE_APP);

    if (__wgetmainargs(&argc, &argv, &env, 0, &start_info) < 0) {
        ExitProcess(-1);
    } else {
        ExitProcess(editor_main(argc, argv));
    }
}

struct MOUSE_CLIC {
    COORD clicCoords;
    INT buttonClic;
};

void spSizeErrorMessage();
void help();
void paintBlack();
void modeCon(int rows, int cols);
void mouse(struct MOUSE_CLIC *mouseClic);
BOOL coordHaveValues(COORD * coord, SHORT X, SHORT Y);
void showCursor(BOOL show);
void loadFromFile(HANDLE hFile);

#define S(color, character) ( (CHAR_INFO){(wchar_t)character, (WORD)color} )
#define SPACE S(0x0F, '-')

#define FULL_BLOCK    0x2588
#define LIGHT_SHADE    0x2591
#define MEDIUM_SHADE   0x2592
#define DARK_SHADE    0x2593
#define WHITE_FACE    0x263A
#define BLACK_FACE    0x263B
#define HEART_SUIT    0x2665
#define DIAMOND_SUIT   0x2666
#define SPADE_SUIT    0x2660
#define CLUB_SUIT    0x2663
#define BEAMED_EIGHTH   0x266B
#define SUN_RAYS    0x263C
#define ASTERISK    0x002A
#define BLACK_SQUARE   0x25A0
#define BLACK_LEFT_POINTING  0x25C4
#define BLACK_RIGHT_POINTING 0x25BA
#define BLACK_UP_POINTING  0x25B2
#define BLACK_DOWN_POINTING  0x25BC

/*
#ifdef USE_ANSI
#define FULL_BLOCK				0xDB
#define LIGHT_SHADE				0xB0
#define MEDIUM_SHADE			0xB1
#define DARK_SHADE				0xB2
#define WHITE_FACE				0x01
#define BLACK_FACE				0x02
#define HEART_SUIT				0x03
#define DIAMOND_SUIT			0x04
#define SPADE_SUIT				0x06
#define CLUB_SUIT				0x05
#define BEAMED_EIGHTH			0x07
#define SUN_RAYS				0x0F
#define ASTERISK				0x2A
#define BLACK_SQUARE			0xFE
#define BLACK_LEFT_POINTING		0x11
#define BLACK_RIGHT_POINTING	0x10
#define BLACK_UP_POINTING		0x1E
#define BLACK_DOWN_POINTING		0x1F
#endif
 */

#define WBlock S(0x0F, MEDIUM_SHADE)
#define BBlock S(0x07, FULL_BLOCK)

#define CBlock S(0x00, 0x20)
#define PBlock S(0x07, 'X')

#define PAINTZONE_MAX_WIDTH (120)
#define PAINTZONE_MAX_HEIGHT (60)

void spSizeErrorMessage() {
    fwprintf(stderr,
            L"Size width must be from 1 to 120 and height must be from 1 to 60\n");
}

void help() {
    fwprintf(stdout,
            L"Bg Sprite Editor 1.0.1\n"
            " Parameters:\n"
            "  EDITOR create filename width eight\n"
            "  EDITOR alter filename\n\n");
}

int editor_main(int argc, wchar_t ** argv) {
#define PALETTE_WIDTH (5)
#define PALETTE_EIGHT (14)
#define CARTOONS_WIDTH (2)
#define CARTOONS_EIGHT (9)

#define BF(b,f) ((b<<4)+f)
#define TBF(c) ((c<<4)+c)

    struct MOUSE_CLIC mouseClic;
    HANDLE spriteFile;

    //Init values
    UINT BColor = 0x0;
    UINT FColor = 0xE;
    UINT Cartoon = BLACK_SQUARE;

    CHAR_INFO palette[PALETTE_EIGHT][PALETTE_WIDTH] = {
        {BBlock, BBlock, BBlock, BBlock, BBlock}
        ,
        {WBlock, WBlock, WBlock, WBlock, WBlock}
        ,
        {WBlock, S(0x00, ' '), WBlock, S(0x80, ' '), WBlock}
        ,
        {WBlock, S(0x10, ' '), WBlock, S(0x90, ' '), WBlock}
        ,
        {WBlock, S(0x20, ' '), WBlock, S(0xA0, ' '), WBlock}
        ,
        {WBlock, S(0x30, ' '), WBlock, S(0xB0, ' '), WBlock}
        ,
        {WBlock, S(0x40, ' '), WBlock, S(0xC0, ' '), WBlock}
        ,
        {WBlock, S(0x50, ' '), WBlock, S(0xD0, ' '), WBlock}
        ,
        {WBlock, S(0x60, ' '), WBlock, S(0xE0, ' '), WBlock}
        ,
        {WBlock, S(0x70, ' '), WBlock, S(0xF0, ' '), WBlock}
        ,
        {WBlock, WBlock, WBlock, WBlock, WBlock}
        ,
        {WBlock, WBlock, WBlock, WBlock, WBlock}
        ,
        {WBlock, WBlock, CBlock, WBlock, WBlock}
        ,
        {WBlock, WBlock, WBlock, WBlock, WBlock}
    };

    CHAR_INFO cartoons[CARTOONS_EIGHT][CARTOONS_WIDTH] = {
        {S(0x07, WHITE_FACE), S(0x07, BLACK_FACE)}
        ,
        {S(0x07, HEART_SUIT), S(0x07, DIAMOND_SUIT)}
        ,
        {S(0x07, CLUB_SUIT), S(0x07, SPADE_SUIT)}
        ,
        {S(0x07, BEAMED_EIGHTH), S(0x07, SUN_RAYS)}
        ,
        {S(0x07, LIGHT_SHADE), S(0x07, FULL_BLOCK)}
        ,
        {S(0x07, MEDIUM_SHADE), S(0x07, BLACK_SQUARE)}
        ,
        {S(0x07, DARK_SHADE), S(0x07, ASTERISK)}
        ,
        {S(0x07, BLACK_RIGHT_POINTING), S(0x07, BLACK_DOWN_POINTING)}
        ,
        {S(0x07, BLACK_UP_POINTING), S(0x07, BLACK_LEFT_POINTING)}
        ,
    };

    COORD BCoord = {3, 14};
    COORD FCoord = {9, 14};
    COORD CCoord = {14, 14};
    DWORD numberOfCharsWritten;
    DWORD numberOfBytesWritten;


    wchar_t *fileName;

    COORD dwSize;

    COORD dwCoord;
    HANDLE hOut;
    SMALL_RECT screenRegion;
    SMALL_RECT paintZoneRegion;


    CHAR_INFO paintCell = PBlock;


    INT i;
    INT limit;
    INT writeErrors;
#define ACTION_NONE  0
#define ACTION_CREATE 1
#define ACTION_OPEN  2
    INT action = ACTION_NONE;


    INT win_width = 100;
    INT win_height = 30;

    if (paintZone_Width > win_width) {
        win_width = 140;
    }

    if (paintZone_Height > win_height) {
        win_height = 63;
    }

    if (argc < 2) {
        help();
        fwprintf(stderr, L"Few arguments.\n");
        return 1;
    }

    modeCon(win_width, win_height);
    paintBlack();

    if (!_wcsicmp(L"CREATE", argv[1])) {
        if (argc != 5) {
            fwprintf(stderr, L"Many arguments.\n");
            return 1;
        }
        action = ACTION_CREATE;
    } else if (!_wcsicmp(L"ALTER", argv[1])) {
        if (argc != 3) {
            fwprintf(stderr, L"Many arguments.\n");
            return 1;
        }
        action = ACTION_OPEN;
    }

    if (ACTION_NONE == action) {
        fwprintf(stderr, L"Invalid command.\n");
        return 1;
    }

    if (ACTION_CREATE == action) {
        fileName = argv[2];
        paintZone_Width = _wtoi(argv[3]);
        paintZone_Height = _wtoi(argv[4]);

        if (paintZone_Width < 1 || paintZone_Width > PAINTZONE_MAX_WIDTH) {
            help();
            spSizeErrorMessage();
            return 1;
        }

        if (paintZone_Height < 1
                || paintZone_Height > PAINTZONE_MAX_HEIGHT) {
            help();
            spSizeErrorMessage();
            return 1;
        }

        spriteFile = CreateFileW(fileName,
                GENERIC_WRITE,
                0,
                NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
                NULL);

        if (INVALID_HANDLE_VALUE == spriteFile) {
            help();
            fwprintf(stderr, L"The file cannot be open for write.\n");
            if (ERROR_FILE_EXISTS == GetLastError()) {
                fwprintf(stderr, L"The file already exists.\n");
            }
            return 1;
        }

        paintZone_Length = (paintZone_Width * paintZone_Height);

        for (i = 0; i < paintZone_Length; ++i) {
            paintZone[i] = paintCell;
        }

    }

    if (ACTION_OPEN == action) {

        fileName = argv[2];

        spriteFile = CreateFileW(fileName,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL, OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL, NULL);

        if (INVALID_HANDLE_VALUE == spriteFile) {
            help();
            fwprintf(stderr, L"The file cannot be open for modify.\n");
            if (ERROR_FILE_NOT_FOUND == GetLastError()) {
                fwprintf(stderr, L"The file not exists.\n");
            }
            return 1;
        }

        loadFromFile(spriteFile);

    }

    showCursor(FALSE);

    dwCoord.X = 0;
    dwCoord.Y = 0;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    //Draw palette foreground and background
    dwSize.X = PALETTE_WIDTH;
    dwSize.Y = PALETTE_EIGHT;
    screenRegion.Left = 1;
    screenRegion.Top = 2;
    screenRegion.Right = screenRegion.Left + dwSize.X - 1;
    screenRegion.Bottom = screenRegion.Top + dwSize.Y - 1;
    WriteConsoleOutputW(hOut,
            (const CHAR_INFO *) palette,
            dwSize, dwCoord, &screenRegion);
    screenRegion.Left = 7;
    screenRegion.Top = 2;
    screenRegion.Right = screenRegion.Left + dwSize.X - 1;
    screenRegion.Bottom = screenRegion.Top + dwSize.Y - 1;
    WriteConsoleOutputW(hOut,
            (const CHAR_INFO *) palette,
            dwSize, dwCoord, &screenRegion);

    //Draw cartoons
    dwSize.X = CARTOONS_WIDTH;
    dwSize.Y = CARTOONS_EIGHT;
    screenRegion.Left = 14;
    screenRegion.Top = 2;
    screenRegion.Right = screenRegion.Left + dwSize.X - 1;
    screenRegion.Bottom = screenRegion.Top + dwSize.Y - 1;
    WriteConsoleOutputW(hOut,
            (const CHAR_INFO *) cartoons,
            dwSize, dwCoord, &screenRegion);


    //Draw paint zone
    dwSize.X = paintZone_Width;
    dwSize.Y = paintZone_Height;
    paintZoneRegion.Left = 18;
    paintZoneRegion.Top = 2;
    paintZoneRegion.Right = paintZoneRegion.Left + dwSize.X - 1;
    paintZoneRegion.Bottom = paintZoneRegion.Top + dwSize.Y - 1;
    WriteConsoleOutputW(hOut,
            (const CHAR_INFO *) paintZone,
            dwSize, dwCoord, &paintZoneRegion);

    FillConsoleOutputAttribute(hOut,
            TBF(BColor), 1, BCoord,
            &numberOfCharsWritten);

    FillConsoleOutputAttribute(hOut,
            TBF(FColor), 1, FCoord,
            &numberOfCharsWritten);

    FillConsoleOutputAttribute(hOut,
            BF(BColor, FColor), 1, CCoord,
            &numberOfCharsWritten);

    FillConsoleOutputCharacterW(hOut,
            Cartoon, 1, CCoord, &numberOfCharsWritten);


Inputs:
    mouse(&mouseClic);

    if (coordHaveValues(&mouseClic.clicCoords, 2, 4))
        BColor = 0x0;
    else if (coordHaveValues(&mouseClic.clicCoords, 2, 5))
        BColor = 0x1;
    else if (coordHaveValues(&mouseClic.clicCoords, 2, 6))
        BColor = 0x2;
    else if (coordHaveValues(&mouseClic.clicCoords, 2, 7))
        BColor = 0x3;
    else if (coordHaveValues(&mouseClic.clicCoords, 2, 8))
        BColor = 0x4;
    else if (coordHaveValues(&mouseClic.clicCoords, 2, 9))
        BColor = 0x5;
    else if (coordHaveValues(&mouseClic.clicCoords, 2, 10))
        BColor = 0x6;
    else if (coordHaveValues(&mouseClic.clicCoords, 2, 11))
        BColor = 0x7;
    else if (coordHaveValues(&mouseClic.clicCoords, 4, 4))
        BColor = 0x8;
    else if (coordHaveValues(&mouseClic.clicCoords, 4, 5))
        BColor = 0x9;
    else if (coordHaveValues(&mouseClic.clicCoords, 4, 6))
        BColor = 0xA;
    else if (coordHaveValues(&mouseClic.clicCoords, 4, 7))
        BColor = 0xB;
    else if (coordHaveValues(&mouseClic.clicCoords, 4, 8))
        BColor = 0xC;
    else if (coordHaveValues(&mouseClic.clicCoords, 4, 9))
        BColor = 0xD;
    else if (coordHaveValues(&mouseClic.clicCoords, 4, 10))
        BColor = 0xE;
    else if (coordHaveValues(&mouseClic.clicCoords, 4, 11))
        BColor = 0xF;

    if (coordHaveValues(&mouseClic.clicCoords, 8, 4))
        FColor = 0x0;
    else if (coordHaveValues(&mouseClic.clicCoords, 8, 5))
        FColor = 0x1;
    else if (coordHaveValues(&mouseClic.clicCoords, 8, 6))
        FColor = 0x2;
    else if (coordHaveValues(&mouseClic.clicCoords, 8, 7))
        FColor = 0x3;
    else if (coordHaveValues(&mouseClic.clicCoords, 8, 8))
        FColor = 0x4;
    else if (coordHaveValues(&mouseClic.clicCoords, 8, 9))
        FColor = 0x5;
    else if (coordHaveValues(&mouseClic.clicCoords, 8, 10))
        FColor = 0x6;
    else if (coordHaveValues(&mouseClic.clicCoords, 8, 11))
        FColor = 0x7;
    else if (coordHaveValues(&mouseClic.clicCoords, 10, 4))
        FColor = 0x8;
    else if (coordHaveValues(&mouseClic.clicCoords, 10, 5))
        FColor = 0x9;
    else if (coordHaveValues(&mouseClic.clicCoords, 10, 6))
        FColor = 0xA;
    else if (coordHaveValues(&mouseClic.clicCoords, 10, 7))
        FColor = 0xB;
    else if (coordHaveValues(&mouseClic.clicCoords, 10, 8))
        FColor = 0xC;
    else if (coordHaveValues(&mouseClic.clicCoords, 10, 9))
        FColor = 0xD;
    else if (coordHaveValues(&mouseClic.clicCoords, 10, 10))
        FColor = 0xE;
    else if (coordHaveValues(&mouseClic.clicCoords, 10, 11))
        FColor = 0xF;

    if (coordHaveValues(&mouseClic.clicCoords, 14, 2))
        Cartoon = WHITE_FACE;
    else if (coordHaveValues(&mouseClic.clicCoords, 14, 3))
        Cartoon = HEART_SUIT;
    else if (coordHaveValues(&mouseClic.clicCoords, 14, 4))
        Cartoon = CLUB_SUIT;
    else if (coordHaveValues(&mouseClic.clicCoords, 14, 5))
        Cartoon = BEAMED_EIGHTH;
    else if (coordHaveValues(&mouseClic.clicCoords, 14, 6))
        Cartoon = LIGHT_SHADE;
    else if (coordHaveValues(&mouseClic.clicCoords, 14, 7))
        Cartoon = MEDIUM_SHADE;
    else if (coordHaveValues(&mouseClic.clicCoords, 14, 8))
        Cartoon = DARK_SHADE;
    else if (coordHaveValues(&mouseClic.clicCoords, 14, 9))
        Cartoon = BLACK_RIGHT_POINTING;
    else if (coordHaveValues(&mouseClic.clicCoords, 14, 10))
        Cartoon = BLACK_UP_POINTING;

    if (coordHaveValues(&mouseClic.clicCoords, 15, 2))
        Cartoon = BLACK_FACE;
    else if (coordHaveValues(&mouseClic.clicCoords, 15, 3))
        Cartoon = DIAMOND_SUIT;
    else if (coordHaveValues(&mouseClic.clicCoords, 15, 4))
        Cartoon = SPADE_SUIT;
    else if (coordHaveValues(&mouseClic.clicCoords, 15, 5))
        Cartoon = SUN_RAYS;
    else if (coordHaveValues(&mouseClic.clicCoords, 15, 6))
        Cartoon = FULL_BLOCK;
    else if (coordHaveValues(&mouseClic.clicCoords, 15, 7))
        Cartoon = BLACK_SQUARE;
    else if (coordHaveValues(&mouseClic.clicCoords, 15, 8))
        Cartoon = ASTERISK;
    else if (coordHaveValues(&mouseClic.clicCoords, 15, 9))
        Cartoon = BLACK_DOWN_POINTING;
    else if (coordHaveValues(&mouseClic.clicCoords, 15, 10))
        Cartoon = BLACK_LEFT_POINTING;


    FillConsoleOutputAttribute(hOut,
            TBF(BColor), 1, BCoord,
            &numberOfCharsWritten);

    FillConsoleOutputAttribute(hOut,
            TBF(FColor), 1, FCoord,
            &numberOfCharsWritten);

    FillConsoleOutputAttribute(hOut,
            BF(BColor, FColor), 1, CCoord,
            &numberOfCharsWritten);

    FillConsoleOutputCharacterW(hOut,
            Cartoon, 1, CCoord, &numberOfCharsWritten);

    //Inside paint zone
    if (mouseClic.clicCoords.X >= paintZoneRegion.Left
            && mouseClic.clicCoords.X <= paintZoneRegion.Right
            && mouseClic.clicCoords.Y >= paintZoneRegion.Top
            && mouseClic.clicCoords.Y <= paintZoneRegion.Bottom) {


        //secondary click: erase
        if (2 == mouseClic.buttonClic) {
            FillConsoleOutputAttribute(hOut,
                    0x07, 1, mouseClic.clicCoords,
                    &numberOfCharsWritten);

            FillConsoleOutputCharacterW(hOut,
                    'X', 1, mouseClic.clicCoords,
                    &numberOfCharsWritten);

        } else {
            FillConsoleOutputAttribute(hOut,
                    BF(BColor, FColor), 1,
                    mouseClic.clicCoords,
                    &numberOfCharsWritten);

            if (BColor == FColor) {
                FillConsoleOutputCharacterW(hOut,
                        FULL_BLOCK, 1,
                        mouseClic.clicCoords,
                        &numberOfCharsWritten);
            } else {
                FillConsoleOutputCharacterW(hOut,
                        Cartoon, 1,
                        mouseClic.clicCoords,
                        &numberOfCharsWritten);
            }
        }
    } else {
        //secondary outside paintzone for quit
        if (2 == mouseClic.buttonClic)
            goto bye;
    }

    goto Inputs;

    return 0;

bye:
    if (0 == ReadConsoleOutputW(hOut,
            (CHAR_INFO *) paintZone,
            dwSize, dwCoord, &paintZoneRegion)) {
        fwprintf(stderr, L"Read failed. Error:%ld\n", GetLastError());
        goto quit;
    }
    //Set transparency
    limit = (dwSize.X * dwSize.Y);
    for (i = 0; i < limit; i++) {
        if (L'X' == paintZone[i].Char.UnicodeChar) {
            paintZone[i].Char.UnicodeChar = 0;
            paintZone[i].Attributes = 0;
        }
    }

    //Begin the save of the file
    writeErrors = 0;

    //save size
    if (0 == WriteFile(spriteFile, &dwSize,
            (sizeof (COORD)), &numberOfBytesWritten, NULL)) {
        ++writeErrors;
    }
    //save data
    if (0 == WriteFile(spriteFile, paintZone, (paintZone_Length << 2), // paintZone_Length * 4 (sizeof(CHAR_INFO))
            &numberOfBytesWritten, NULL)) {
        ++writeErrors;
    }

    if (writeErrors) {
        fwprintf(stderr, L"Write failed. Error:%ld\n", GetLastError());

    } else {
        paintBlack();
        fwprintf(stdout, L"Saved correctly to: %ls\n", fileName);

    }


quit:
    CloseHandle(spriteFile);

    showCursor(TRUE);

    return 0;

}

BOOL coordHaveValues(COORD * coord, SHORT X, SHORT Y) {
    if ((coord->X == X) && (coord->Y == Y)) {
        return TRUE;
    }

    return FALSE;
}

void paintBlack() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbInfo;
    DWORD nLength;
    COORD coord;
    DWORD numberOfCharsWritten;

    GetConsoleScreenBufferInfo(hOut, &csbInfo);

    nLength = csbInfo.dwSize.X * csbInfo.dwSize.Y;
    coord.X = 0;
    coord.Y = 0;

    SetConsoleCursorPosition(hOut, coord);

    FillConsoleOutputAttribute(hOut,
            0x00, nLength, coord,
            &numberOfCharsWritten);

    FillConsoleOutputCharacterW(hOut,
            0x20, nLength, coord,
            &numberOfCharsWritten);
}

void modeCon(int cols, int rows) {

    HANDLE hOut;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    SMALL_RECT windowSize;
    COORD dwSize;

    dwSize.X = cols;
    dwSize.Y = rows;

    windowSize.Left = 0;
    windowSize.Top = 0;
    windowSize.Right = dwSize.X - 1;
    windowSize.Bottom = dwSize.Y - 1;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(hOut, &csbi);
    SetConsoleScreenBufferSize(hOut, csbi.dwMaximumWindowSize);
    SetConsoleWindowInfo(hOut, TRUE, &windowSize);
    SetConsoleScreenBufferSize(hOut, dwSize);
    SetConsoleWindowInfo(hOut, TRUE, &windowSize);


}

void mouse(struct MOUSE_CLIC *mouseClic) {
#ifndef ENABLE_QUICK_EDIT_MODE
#define ENABLE_QUICK_EDIT_MODE 0x0040
#endif

#ifndef ENABLE_EXTENDED_FLAGS
#define ENABLE_EXTENDED_FLAGS 0x0080
#endif
    HANDLE hIn;
    DWORD old_mode;
    DWORD new_mode;
    DWORD readed;
    INPUT_RECORD ir[1];
    int clic;

    hIn = CreateFileW(L"CONIN$",
            (GENERIC_READ | GENERIC_WRITE),
            (FILE_SHARE_READ | FILE_SHARE_WRITE),
            NULL, OPEN_EXISTING, 0, NULL);

    if (INVALID_HANDLE_VALUE == hIn) {
        mouseClic->clicCoords.X = -1;
        mouseClic->clicCoords.Y = -1;
        mouseClic->buttonClic = -1;
        return;
    }

    GetConsoleMode(hIn, &old_mode);

    //
    // keep current configuration,
    // but enable mouse input,
    // disable quick edit mode,
    // disable ctrl+c
    //
    new_mode =
            ((old_mode | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS) &
            ~(ENABLE_QUICK_EDIT_MODE | ENABLE_PROCESSED_INPUT));
    SetConsoleMode(hIn, new_mode);

mouse_in:
    ReadConsoleInputW(hIn, ir, 1, &readed);
    if (MOUSE_EVENT == ir[0].EventType) {
        clic = (int) (ir[0].Event.MouseEvent.dwButtonState);

        // allow only primary button and secondary button
        switch (clic) {
            case FROM_LEFT_1ST_BUTTON_PRESSED:
            case RIGHTMOST_BUTTON_PRESSED:
                mouseClic->clicCoords.X =
                        ir[0].Event.MouseEvent.dwMousePosition.X;
                mouseClic->clicCoords.Y =
                        ir[0].Event.MouseEvent.dwMousePosition.Y;
                mouseClic->buttonClic = clic;
                goto mouse_end;
        }
    }
    goto mouse_in;

mouse_end:
    // restore mouse configuration
    SetConsoleMode(hIn, old_mode);
    CloseHandle(hIn);

}

void showCursor(BOOL show) {
    HANDLE hOut;
    CONSOLE_CURSOR_INFO cci;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(hOut, &cci);

    if (show) {
        cci.bVisible = TRUE;
    } else {
        cci.bVisible = FALSE;
    }

    SetConsoleCursorInfo(hOut, &cci);
}

void loadFromFile(HANDLE hFile) {
    INT i;
    DWORD read;
    DWORD count;
    BOOL ret;
    COORD size;
    CHAR_INFO paintCell = PBlock;

    /* get the dword structure at the beginning of file */
    ret = ReadFile(hFile, &size, sizeof (COORD), &read, NULL);

    if ((read != sizeof (COORD)) || !ret) {

        /* the read failed */
        fwprintf(stderr, L"Invalid file format.\n");
        ExitProcess(1);

    }

    paintZone_Height = size.Y;
    paintZone_Width = size.X;

    if (paintZone_Width < 1 || paintZone_Width > PAINTZONE_MAX_WIDTH) {
        fwprintf(stderr, L"Invalid file format.\n");
        ExitProcess(1);
    }

    if (paintZone_Height < 1 || paintZone_Height > PAINTZONE_MAX_HEIGHT) {
        fwprintf(stderr, L"Invalid file format.\n");
        ExitProcess(1);
    }

    paintZone_Length = (paintZone_Width * paintZone_Height);
    count = paintZone_Length << 2; //= paintZone_Length * 4 (sizeof(CHAR_INFO))

    ret = ReadFile(hFile, paintZone, count, &read, NULL);

    if (!ret || (read != count)) {

        /* the read failed */
        fwprintf(stderr, L"Invalid file format.\n");
        ExitProcess(1);

    }

    /* try to read more, to check the file is conform
       if there are more bytes means that the coord size is incorrect
     */

    ReadFile(hFile, &i, sizeof (int), &read, NULL);

    if (read != 0) {

        fwprintf(stderr, L"Invalid file format.\n");
        ExitProcess(1);
    }

    SetFilePointer(hFile, 0, NULL, FILE_BEGIN);

    /* convert '\0' characters to 'X' */
    for (i = 0; i < paintZone_Length; ++i) {
        if (!paintZone[i].Char.UnicodeChar) {
            paintZone[i] = paintCell;
        }
    }

}
