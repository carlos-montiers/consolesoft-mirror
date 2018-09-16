/*
  Copyright (C) 2010-2018 Carlos Montiers Aguilera

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
 * BG version v3.9 se
 * For compilation with tdm-gcc :
 * gcc -Wl,-e,__start -nostartfiles -m32 -O3 -s bg.c -lwinmm -o BG.EXE -fno-omit-frame-pointer -flto -Wall
 */

#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <locale.h>

// Undocumented functions and structures
// BEGIN
BOOL WINAPI SetConsoleFont(HANDLE hConsoleOutput, DWORD nFont);
// END

typedef BOOL(WINAPI * Func_SetCurrentConsoleFontEx) (HANDLE, BOOL,
        PCONSOLE_FONT_INFOEX);

typedef struct {
    int newmode;
} _startupinfo;

int __wgetmainargs(int *_Argc, wchar_t *** _Argv, wchar_t *** _Env,
        int _DoWildCard, _startupinfo * _StartInfo);

void bg_main(void);

typedef struct _fcprint_info {
    SMALL_RECT sr;
    CHAR_INFO ci;
    HANDLE hOut;
    COORD screen_max;
    COORD size;
    COORD coord;
    SHORT fixed_column;
} FCPRINT_INFO;

//Global variables
INT argc;
WCHAR **argv;
WCHAR **env;
INT ret = 0;

FCPRINT_INFO fcprint_info;

void _start(void) {
    _startupinfo start_info = {0};

    if (__wgetmainargs(&argc, &argv, &env, 0, &start_info) < 0) {
        ExitProcess(-1);
    } else {
        bg_main();
        ExitProcess(ret);
    }
}

void print(void);
void fcprint(void);
void wfcputchar(WCHAR chr);
void wputchar(WCHAR chr);
void color(void);
void locate(void);
void last_kbd(void);
void kbd(void);
UINT keystick();
void mouse(void);
void date_time(void);
void sleep(void);
void cursor(void);
void font(void);
WCHAR convert_extended_oem_to_unicode(INT num);
void process_escape_sequences(WCHAR * str, void (*callBack)(WCHAR));
void play(void);
HANDLE get_output_handle(void);
HANDLE get_input_handle(void);

typedef void (*Function) (void);

typedef struct _func {
    WCHAR *name;
    Function func;
} FUNC;

FUNC functions[] = {
    {TEXT("PRINT"), (Function) print},
    {TEXT("FCPRINT"), (Function) fcprint},
    {TEXT("COLOR"), (Function) color},
    {TEXT("LOCATE"), (Function) locate},
    {TEXT("LASTKBD"), (Function) last_kbd},
    {TEXT("KBD"), (Function) kbd},
    {TEXT("MOUSE"), (Function) mouse},
    {TEXT("DATETIME"), (Function) date_time},
    {TEXT("SLEEP"), (Function) sleep},
    {TEXT("CURSOR"), (Function) cursor},
    {TEXT("FONT"), (Function) font},
    {TEXT("PLAY"), (Function) play}
};

void bg_main() {

    WCHAR *name_function;
    INT i;
    INT limit = (sizeof (functions) / sizeof (*functions));

    if (argc >= 2) {

        name_function = argv[1];

        for (i = 0; i < limit; ++i) {
            if (!_wcsicmp(name_function, functions[i].name)) {
                functions[i].func();
                break;
            }
        }
    }
}

void print(void) {

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut;
    WCHAR *text;
    INT arg_index;
    INT limit;
    WORD color;

    // Sets the locale to the default,
    // which is the user-default ANSI code page
    // obtained from the operating system.
    setlocale(LC_ALL, "");
    // Set "stdout" to have unicode mode:
    _setmode(_fileno(stdout), _O_U16TEXT);

    if (3 == argc) {

        text = argv[2];
        process_escape_sequences(text, wputchar);

    } else if (argc >= 4) {

        arg_index = 2; // Begin in argv[2]
        limit = argc - 1;

        hOut = get_output_handle();
        GetConsoleScreenBufferInfo(hOut, &csbi);

        while (arg_index < limit) {
            color = (WORD) wcstol(argv[arg_index++], (WCHAR **) NULL, 16);
            text = argv[arg_index++];
            SetConsoleTextAttribute(hOut, color);
            process_escape_sequences(text, wputchar);
        }
        SetConsoleTextAttribute(hOut, csbi.wAttributes);

        CloseHandle(hOut);
    }
}

void fcprint(void) {

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut;
    WCHAR *text;
    COORD coord;
    INT arg_index;
    INT limit;

    if (argc >= 6) {

        arg_index = 2; // Begin in argv[2]
        limit = argc - 1;

        hOut = get_output_handle();
        GetConsoleScreenBufferInfo(hOut, &csbi);

        coord.Y = wcstol(argv[arg_index++], (WCHAR **) NULL, 10);
        coord.X = wcstol(argv[arg_index++], (WCHAR **) NULL, 10);

        // init_fcprint_info data
        fcprint_info.hOut = hOut;
        fcprint_info.sr.Left = coord.X;
        fcprint_info.sr.Top = coord.Y;
        fcprint_info.fixed_column = coord.X;
        fcprint_info.screen_max.X = csbi.srWindow.Right - csbi.srWindow.Left;
        fcprint_info.screen_max.Y = csbi.srWindow.Bottom - csbi.srWindow.Top;
        fcprint_info.size.X = 1;
        fcprint_info.size.Y = 1;
        fcprint_info.coord.X = 0;
        fcprint_info.coord.Y = 0;

        while (arg_index < limit) {
            fcprint_info.ci.Attributes = (WORD) wcstol(argv[arg_index++], (WCHAR **) NULL, 16);
            text = argv[arg_index++];
            process_escape_sequences(text, wfcputchar);
        }
    }
}

void wfcputchar(WCHAR chr) {
#define INVISIBLE_CHAR '\0'
#define NEWLINE_CHAR '\n'

    switch (chr) {

        case INVISIBLE_CHAR:
            fcprint_info.sr.Left += 1;
            break;

        case NEWLINE_CHAR:
            fcprint_info.sr.Top += 1;
            fcprint_info.sr.Left = fcprint_info.fixed_column;

            break;

        default:

            if (fcprint_info.sr.Left >= 0
                    && fcprint_info.sr.Left <= fcprint_info.screen_max.X
                    && fcprint_info.sr.Top >= 0
                    && fcprint_info.sr.Top <= fcprint_info.screen_max.Y
                    ) {
                fcprint_info.ci.Char.UnicodeChar = chr;
                fcprint_info.sr.Right = fcprint_info.sr.Left + 1;
                fcprint_info.sr.Bottom = fcprint_info.sr.Top + 1;
                WriteConsoleOutput(fcprint_info.hOut, &fcprint_info.ci,
                        fcprint_info.size, fcprint_info.coord,
                        &fcprint_info.sr);
            }
            fcprint_info.sr.Left += 1;
            break;

    }
}

void wputchar(WCHAR chr) {

    fputwc(chr, stdout);
}

void color() {

    HANDLE hOut;
    WORD color;

    hOut = get_output_handle();

    if (3 == argc) {
        color = (WORD) wcstol(argv[2], (WCHAR **) NULL, 16);
    } else {
        color = (WORD) 0x07;
    }
    SetConsoleTextAttribute(hOut, color);
    CloseHandle(hOut);
}

void locate(void) {

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hOut;
    COORD screen_max;
    COORD coord;
    SHORT x, y;

    if (4 == argc) {

        hOut = get_output_handle();
        GetConsoleScreenBufferInfo(hOut, &csbi);
        screen_max.X = csbi.srWindow.Right - csbi.srWindow.Left;
        screen_max.Y = csbi.srWindow.Bottom - csbi.srWindow.Top;

        y = wcstol(argv[2], (WCHAR **) NULL, 10);
        x = wcstol(argv[3], (WCHAR **) NULL, 10);

        if (x < 0) {
            x = 0;
        }

        if (y < 0) {
            y = 0;
        }

        if (x > screen_max.X) {
            x = screen_max.X;
        }

        if (y > screen_max.Y) {
            y = screen_max.Y;
        }

        coord.X = x;
        coord.Y = y;

        SetConsoleCursorPosition(hOut, coord);
        CloseHandle(hOut);
    }
}

void last_kbd(void) {
    if (_kbhit()) {
        kbd();
    } else {
        ret = 0;
    }
}

void kbd(void) {

    INT key;

    key = _getch();
    if ((!key) || (0xE0 == key)) {
        key = _getch();
        key += 256;
    }

    ret = key;
}

void mouse(void) {

    INPUT_RECORD ir[1];
    HANDLE hIn;
    DWORD old_mode;
    DWORD new_mode;
    DWORD readed;
    INT row, col;
    INT clic;

    hIn = get_input_handle();

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
    ReadConsoleInput(hIn, ir, 1, &readed);
    if (MOUSE_EVENT == ir[0].EventType) {

        // allow only primary button
        switch (ir[0].Event.MouseEvent.dwButtonState) {
            case FROM_LEFT_1ST_BUTTON_PRESSED:
                row = (INT) ir[0].Event.MouseEvent.dwMousePosition.Y;
                col = (INT) ir[0].Event.MouseEvent.dwMousePosition.X;

                wprintf(L"%d %d\n", row, col);

                clic = row << 0x10;
                clic += col;

                goto mouse_end;
        }
    }
    goto mouse_in;

mouse_end:
    // restore mouse configuration
    SetConsoleMode(hIn, old_mode);
    CloseHandle(hIn);
    ExitProcess(clic);
}

void date_time(void) {

    SYSTEMTIME st;

    GetLocalTime(&st);

    wprintf(L"%d %d %d %d %d %d %d %d\n",
            (int) st.wDayOfWeek,
            (int) st.wYear, (int) st.wMonth, (int) st.wDay,
            (int) st.wHour, (int) st.wMinute, (int) st.wSecond,
            (int) st.wMilliseconds);

}

void sleep(void) {

    LONG dwMilliseconds;

    if (3 == argc) {

        dwMilliseconds = (LONG) wcstol(argv[2], (WCHAR **) NULL, 10);

        if (dwMilliseconds > 0) {
            Sleep(dwMilliseconds);
        }

    }
}

void cursor(void) {

    CONSOLE_CURSOR_INFO cci;
    HANDLE hOut;
    LONG value;

    hOut = get_output_handle();
    GetConsoleCursorInfo(hOut, &cci);

    if (3 == argc) {

        value = (LONG) wcstol(argv[2], (WCHAR **) NULL, 10);

        switch (value) {
            case 0:
                cci.bVisible = FALSE;
                break;
            case 1:
                cci.bVisible = TRUE;
                break;
            case 25:
            case 50:
            case 100:
                cci.dwSize = value;
                cci.bVisible = TRUE;
                break;
        }

    } else {
        cci.bVisible = TRUE;
        cci.dwSize = 25;
    }

    SetConsoleCursorInfo(hOut, &cci);
    CloseHandle(hOut);
}

void font(void) {
#define TERMINAL_FONTS 10

    CONSOLE_FONT_INFOEX font_info;
    COORD terminal_font[] = {
        {4, 6}
        ,
        {6, 8}
        ,
        {8, 8}
        ,
        {16, 8}
        ,
        {5, 12}
        ,
        {7, 12}
        ,
        {8, 12}
        ,
        {16, 12}
        ,
        {12, 16}
        ,
        {10, 18}
    };
    HINSTANCE dllHandle;
    Func_SetCurrentConsoleFontEx SetCurrentConsoleFontEx_Ptr;
    HANDLE hOut;
    INT index;

    if (3 == argc) {

        index = (INT) wcstol(argv[2], (WCHAR **) NULL, 10);
        if ((index < 0) || (index >= TERMINAL_FONTS)) {
            return;
        }

        hOut = get_output_handle();

        dllHandle = LoadLibraryW(L"KERNEL32.DLL");

        if (NULL != dllHandle) {
            SetCurrentConsoleFontEx_Ptr =
                    (Func_SetCurrentConsoleFontEx)
                    GetProcAddress(dllHandle, "SetCurrentConsoleFontEx");

            if (NULL != SetCurrentConsoleFontEx_Ptr) { //vista

                font_info.cbSize = sizeof (CONSOLE_FONT_INFOEX);
                font_info.nFont = index;
                font_info.dwFontSize.X = terminal_font[index].X;
                font_info.dwFontSize.Y = terminal_font[index].Y;
                font_info.FontFamily = 48;
                font_info.FontWeight = 400;
                wcscpy(font_info.FaceName, L"Terminal");

                SetCurrentConsoleFontEx_Ptr(hOut, FALSE, &font_info);
            }

            FreeLibrary(dllHandle);
        }

        SetConsoleFont(hOut, index);
        CloseHandle(hOut);
    }
}

WCHAR convert_extended_oem_to_unicode(INT num) {
    WCHAR wnum;

    if (num >= 128) {
        OemToCharBuffW((LPCSTR) & num, &wnum, 1);
    } else {
        wnum = num;
    }

    return wnum;
}

void process_escape_sequences(WCHAR * str, void (*callBack)(WCHAR)) {
    INT width_num;
    INT using_escape_sequence;
    INT num;
    WCHAR wnum;
    WCHAR letter;

    using_escape_sequence = width_num = num = 0;

    if (!str) {
        return;
    }
#define HEX_WIDTH (2)
#define ESCAPE_SYMBOL  ('\\')
#define XToInt(xdigit)  ( (xdigit <= '9') ? (xdigit - '0') : ((xdigit | 32) - 'W') )

    while ((letter = *str)) {
        if (!using_escape_sequence) {
            if (!(using_escape_sequence = (ESCAPE_SYMBOL == letter))) {
                callBack(letter);
            }
        } else {
            if (iswctype(letter, _HEX)) {
                if (width_num < HEX_WIDTH) {
                    ++width_num;

                    num <<= 4; // multiply num by 16
                    num += XToInt(letter); // sum to num the conversion of hex char to int

                } else {
                    wnum = convert_extended_oem_to_unicode(num);
                    callBack(wnum);
                    callBack(letter);
                    using_escape_sequence = width_num = num = 0;
                }

            } else {
                if (width_num) {
                    wnum = convert_extended_oem_to_unicode(num);
                    callBack(wnum);
                    if (!(using_escape_sequence = (ESCAPE_SYMBOL == letter))) {
                        callBack(letter);
                    }
                    width_num = num = 0;
                } else {
                    switch (letter) {
                        case ('b'):
                            callBack(8);
                            break;

                        case ('r'):
                            callBack(13);
                            break;

                        case ('n'):
                            callBack(10);
                            break;

                        case ('t'):
                            callBack(9);
                            break;

                        default:
                            callBack(letter);
                            break;
                    }
                    using_escape_sequence = 0;
                }
            }

        }
        ++str;
    }

    if (width_num) {
        wnum = convert_extended_oem_to_unicode(num);
        callBack(wnum);
    }
}

void play(void) {

    WCHAR * wavFile;
    INT timesToPlay;
    INT mode;

    if (argc >= 3) {

        wavFile = argv[2];
        timesToPlay = 1;
        mode = SND_FILENAME | SND_SYNC | SND_NODEFAULT | SND_NOSTOP;

        if (argc >= 4) {
            timesToPlay = (int) wcstol(argv[3], (WCHAR **) NULL, 10);
            if (timesToPlay < 0) {
                timesToPlay = 0;
            }
        }

        while (timesToPlay--) {
            PlaySoundW(wavFile, NULL, mode);
        }
    }

}

HANDLE get_output_handle(void) {

    HANDLE hOut;

    hOut = CreateFileW(L"CONOUT$",
            (GENERIC_READ | GENERIC_WRITE),
            (FILE_SHARE_READ | FILE_SHARE_WRITE),
            NULL, OPEN_EXISTING, 0, NULL);

    return hOut;
}

HANDLE get_input_handle(void) {

    HANDLE hIn;

    hIn = CreateFile(TEXT("CONIN$"),
            (GENERIC_READ | GENERIC_WRITE),
            (FILE_SHARE_READ | FILE_SHARE_WRITE),
            NULL, OPEN_EXISTING, 0, NULL);

    return hIn;
}
