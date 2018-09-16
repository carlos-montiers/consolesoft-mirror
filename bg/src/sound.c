/*
  Copyright (C) 2014 Carlos Montiers Aguilera
  Copyright (C) 2014 Jason Hood

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

  Carlos Montiers Aguilera   cmontiers@gmail.com
  Jason Hood                 jadoxa@yahoo.com.au
 */

/*
 * Sound v1.1.1
 * For compilation with tdm-gcc :
 * gcc -Wl,-e,__start -nostartfiles -m32 -O3 -s sound.c -lwinmm -o SOUND.EXE -Wall
 */

/*
 * version 1.1.0 fixes a previous problem happened on windows 7 sp1.
 */

#define UNICODE
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct tagPROCESSENTRY32 {
    DWORD dwSize;
    DWORD cntUsage;
    DWORD th32ProcessID;
    ULONG_PTR th32DefaultHeapID;
    DWORD th32ModuleID;
    DWORD cntThreads;
    DWORD th32ParentProcessID;
    LONG pcPriClassBase;
    DWORD dwFlags;
    WCHAR szExeFile[MAX_PATH];
} PROCESSENTRY32W, *LPPROCESSENTRY32W;

HANDLE WINAPI CreateToolhelp32Snapshot(DWORD dwFlags, DWORD th32ProcessID);
BOOL WINAPI Process32FirstW(HANDLE hSnapshot, LPPROCESSENTRY32W lppe);
BOOL WINAPI Process32NextW(HANDLE hSnapshot, LPPROCESSENTRY32W lppe);
#define TH32CS_SNAPPROCESS 0x2

#ifndef SND_FILENAME
#define SND_FILENAME 0x00020000L
#endif
#ifndef SND_SYNC
#define SND_SYNC 0x0000
#endif
#ifndef SND_ASYNC
#define SND_ASYNC 0x0001
#endif
#ifndef SND_NODEFAULT
#define SND_NODEFAULT 0x0002
#endif
#ifndef SND_LOOP
#define SND_LOOP 8
#endif

typedef struct {
    int newmode;
} _startupinfo;

int __wgetmainargs(int *_Argc, wchar_t *** _Argv, wchar_t *** _Env,
        int _DoWildCard, _startupinfo * _StartInfo);

//BOOL WINAPI PlaySoundW(LPCWSTR, HMODULE, DWORD);

void SetSoundEventName(DWORD ParentPid, wchar_t * fullPathWavFileName);
wchar_t *GetFileName(wchar_t * fullPathFileName);
DWORD GetIdOfParentProcess(void);

//handle for event for stop the sound in the current process
static HANDLE hSoundStop = NULL;

static HANDLE HParentProcess = NULL;

// SoundEventName is a memory zone for hold a event name.
static wchar_t SoundEventName[MAX_PATH] = {0};

static wchar_t *PlaceEvent = L"TechNoir";

DWORD WINAPI Detonator(LPVOID param) {
#define H_ARRAY_SIZE (2)
    INT i;
    DWORD ParentPid;
    HANDLE hArray[H_ARRAY_SIZE];

    //manual reset event, initial state is nonsignaled
    hSoundStop = CreateEventW(NULL, TRUE, FALSE, SoundEventName);
    if (!hSoundStop) {
        fwprintf(stderr, L"Failed to create event. Error: (%lu).\n",
                GetLastError());

        ExitProcess(1);
    }

    ParentPid = _wtol(SoundEventName);
    HParentProcess = OpenProcess(SYNCHRONIZE, FALSE, ParentPid);
    if (!HParentProcess) {
        fwprintf(stderr, L"Failed to open process: %ld. Error: (%lu).\n",
                ParentPid, GetLastError());

        CloseHandle(hSoundStop);
        ExitProcess(1);
    }

    hArray[0] = hSoundStop;
    hArray[1] = HParentProcess;
    WaitForMultipleObjects(H_ARRAY_SIZE, hArray, FALSE, INFINITE);

    for (i = 0; i < H_ARRAY_SIZE; i++) {
        CloseHandle(hArray[i]);
    }

    ExitProcess(0);
}

void SetSoundEventName(DWORD ParentPid, wchar_t * wavFileName) {
    //we use the ; as a number terminator for the _wtol function
    _snwprintf(SoundEventName, MAX_PATH,
            L"%ld;%ls", ParentPid, GetFileName(wavFileName));
    SoundEventName[MAX_PATH - 1] = L'\0';
    //to lower
    _wcslwr(SoundEventName);
}

wchar_t *GetFileName(wchar_t * wavFileName) {
    wchar_t *fileName = wavFileName;
    wchar_t *pi = fileName;

    while (*pi) {
        if ((L'\\' == *pi) || (L'/' == *pi)) {
            fileName = pi + 1;
        }
        ++pi;
    }
    return fileName;
}

void _start() {
    _startupinfo start_info = {0};
    INT argc;
    WCHAR **argv;
    WCHAR **env;

    HANDLE hThread;

    if (__wgetmainargs(&argc, &argv, &env, 0, &start_info) < 0) {
        ExitProcess(-1);
    }

    if ((argc < 3) || (argc > 4)) {
        ExitProcess(1);
    }

    SoundEventName[0] = 0;
    // If not get environment variable "=" as a null-terminated string
    // because is not found :
    // create it and run a new process
    if (0 == GetEnvironmentVariableW(L"=", SoundEventName, MAX_PATH)) {

        PROCESS_INFORMATION pi;
        STARTUPINFOW si;
        HANDLE event;

        event = CreateEventW(NULL, TRUE, FALSE, PlaceEvent);

        memset(&pi, 0, sizeof (pi));
        memset(&si, 0, sizeof (si));
        si.cb = sizeof (si);

        // We create a event name using as prefix the id of the parent process
        // This will prevent when you stop a sound, stop sound started by other cmds
        SetSoundEventName(GetIdOfParentProcess(), argv[2]);
        SetEnvironmentVariableW(L"=", SoundEventName);
        CreateProcessW(NULL, GetCommandLineW(), NULL, NULL, FALSE,
                CREATE_UNICODE_ENVIRONMENT, NULL, NULL, &si, &pi);

        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);

        WaitForSingleObject(event, 1000);

        CloseHandle(event);

        ExitProcess(0);

    }// if SoundEventName is still null because length of environment variable value
        // is greather than MAX_PATH is strange, what other app set this environment variable ?
    else if (0 == SoundEventName[0]) {
        ExitProcess(1);
    }
    //Init detonator thread
    hThread = CreateThread(NULL, 4096, Detonator, NULL, 0, NULL);
    if (!hThread) {
        fwprintf(stderr, L"Failed to create thread. Error: (%lu).\n",
                GetLastError());
        ExitProcess(1);
    }
    CloseHandle(hThread);

    SetEvent(OpenEventW(EVENT_MODIFY_STATE, FALSE, PlaceEvent));

    if (!_wcsicmp(argv[1], L"play")) {

        if (4 == argc) {
            DWORD dwMilliseconds;
            INT duration;

            duration = _wtoi(argv[3]);

            if (duration >= 0) {
                dwMilliseconds = duration * 1000;
            } else if (-1 == duration) {
                dwMilliseconds = INFINITE;
            } else {
                dwMilliseconds = 0;
            }

            PlaySoundW(argv[2], NULL,
                    (SND_ASYNC | SND_LOOP | SND_FILENAME |
                    SND_NODEFAULT));

            Sleep(dwMilliseconds);


        } else {
            PlaySoundW(argv[2], NULL,
                    (SND_SYNC | SND_FILENAME | SND_NODEFAULT));
        }

        //Detonate pressing the stop Button
        SetEvent(hSoundStop);

    } else if (!_wcsicmp(argv[1], L"stop")) {
        //This event is in other process
        HANDLE event =
                OpenEventW(EVENT_MODIFY_STATE, FALSE, SoundEventName);
        if (event) {
            BOOL eventSigned;
            eventSigned = SetEvent(event);
            if (!eventSigned) {
                fwprintf(stderr, L"Failed to sign event. Error: (%lu).\n",
                        GetLastError());
                ExitProcess(1);
            }

        }
    }

    ExitProcess(0);
}

// Obtain the if of the parent process, if fails exit

DWORD GetIdOfParentProcess(void) {
    HANDLE hSnap;
    DWORD currentProcessId;
    PROCESSENTRY32W pe;
    BOOL fOk;

    hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE) {
        fwprintf(stderr, L"Failed to create snapshot. Error: (%lu).\n",
                GetLastError());
        ExitProcess(1);
    }

    currentProcessId = GetCurrentProcessId();

    pe.dwSize = sizeof (PROCESSENTRY32W);

    for (fOk = Process32FirstW(hSnap, &pe); fOk;
            fOk = Process32NextW(hSnap, &pe)) {
        if (pe.th32ProcessID == currentProcessId) {
            break;
        }
    }
    CloseHandle(hSnap);
    if (!fOk) {
        fwprintf(stderr, L"Failed to locate parent.\n");
        ExitProcess(1);
    }

    return (pe.th32ParentProcessID);
}
