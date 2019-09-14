/*
  Copyright (C) 2010-2019 Carlos Montiers Aguilera

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
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

#define bhx_version "5.9"

/*
 * BHX v5.9
 *
 * Compilation with TCC :
 * tcc bhx.c crt1.c
 * Compilation with tdm-gcc :
 * gcc -Wl,-e,__start -nostartfiles -m32 -Os -s bhx.c crt1.c -o bhx.exe -fno-omit-frame-pointer -flto -Wall
 */

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#include <conio.h>

struct BUFFER {
    BYTE *data;
    DWORD size;
};

static char map_enc85[85 + 1] = {
    "0123456789"
    "abcdefghij"
    "klmnopqrst"
    "uvwxyzABCD"
    "EFGHIJKLMN"
    "OPQRSTUVWX"
    "YZ.-:+=^`/"
    "*?&<>()[]{"
    "}~,$#"
};

int bhx(struct BUFFER buffer, char *sfileName, BOOL bHexEncode, FILE *fFileOutput);
int PrintRebuildFunction(FILE * fileOutput);
char *getFileName(char *);
struct BUFFER file2buffer(char *);
int help(void);
BOOL isCabinet(struct BUFFER *buffer);
int main(int argc, char *argv[]);

int help(void) {
    printf("\nBHX " bhx_version "\n"
            "Encode a binary file in a batch script for rebuild it.\n"
            "\n"
            "By default, the generated batch script name is mybin.cmd.\n"
            "Is recommended convert your file in a cabinet\n"
            "with compression, created with this command line:\n"
            "Makecab /d compressiontype=lzx file file.cab\n"
            "and use the generated cabinet as source.\n"
            "This can save many bytes in the output script.\n"
            "\n"
            "BHX source ... [/hex] [/o:out] [/y]\n"
            "\n"
            "  source  Binary to encode.\n"
            "  /hex    Encode binary data using 16 hexadecimal characters.\n"
            "          The default encoder uses 85 printable characters.\n"
            "  /o:out  out is the output filename instead of mybin.cmd.\n"
            "  /y      Overwrite the output file.\n"
            "\n"
            "BHX /author\n"
            "  display the author of the program.\n" "\n");

    return 0;
}

int main(int argc, char *argv[]) {

    /* Default options */
    char *sOuputFileName = "mybin.cmd"; /* Default output filename */
    BOOL bOverwrite = FALSE; /* Overwrite ouput file */
    BOOL bHexEncode = FALSE; /* Use enc85 as default */

    struct BUFFER buffer;
    BOOL bIsCabinet;

    long i;
    char *sArg;
    char *sPathFile;
    char *sFileName;


    BOOL bArgumentError = FALSE;
    int iNbSources = 0;
    BOOL bFileOutputExists;
    BOOL bFileInputCannotRead;
    FILE *fFileOutput;
    FILE *fFileInput;

    if (argc == 2) {
        if (!stricmp(argv[1], "/author")) {
            printf("\nBHX was programmed by Carlos Montiers Aguilera.\n\n");
            return 0;
        }
        if (!stricmp(argv[1], "/?")) {
            return help();
        }
    }

#define OPT_OUT_FN_PREFIX_LENGTH 3 /* The length of /o: */

    /* Read options */
    for (i = 1; i < argc && !bArgumentError; i++) {
        sArg = argv[i];
        if (stricmp(sArg, "/y") == 0) {
            bOverwrite = TRUE;
        } else if (stricmp(sArg, "/hex") == 0) {
            bHexEncode = TRUE;
        } else if (strnicmp(sArg, "/o:", OPT_OUT_FN_PREFIX_LENGTH) == 0) {
            sOuputFileName = sArg + OPT_OUT_FN_PREFIX_LENGTH;
        } else if (strnicmp(sArg, "/", 1) != 0) {
            iNbSources++;
        } else {
            bArgumentError = TRUE;
        }
    }

    if (bArgumentError) {
        printf("Error: Invalid argument: %s\n", sArg);
        return 1;
    }

    if (iNbSources == 0) {
        return help();
    }

    /* Check for Prerequisites */
    for (i = 1; i < argc; i++) {
        sArg = argv[i];
        if (strnicmp(sArg, "/", 1) != 0) {
            sPathFile = sArg;
            fFileInput = fopen(sPathFile, "rb");
            bFileInputCannotRead = FALSE;
            if (NULL == fFileInput) {
                bFileInputCannotRead = TRUE;
            } else {
                fclose(fFileInput);
            }
            if (bFileInputCannotRead) {
                printf("Error: Source: \"%s\" cannot be opened for reading.\n", sPathFile);
                return 1;
            }
        }
    }
    bFileOutputExists = (-1 != access(sOuputFileName, 0));
    if (bFileOutputExists && !bOverwrite) {
        /* Prompt for overwrite */
        int goOut = FALSE;
        int reply;

        while (!goOut) {
            printf("Overwrite \"%s\"? (y/n): ", sOuputFileName);
            reply = getch();
            printf("%c\n", reply);
            switch (reply) {
                case 'Y':
                case 'y':
                    bOverwrite = TRUE;
                    goOut = TRUE;
                    break;
                case 'N':
                case 'n':
                    bOverwrite = FALSE;
                    goOut = TRUE;
                    break;
            }
        }

        if (!bOverwrite) {
            return 1;
        }
    }
    fFileOutput = fopen(sOuputFileName, "w");
    if (NULL == fFileOutput) {
        printf("Error: \"%s\" cannot be opened for writing.\n", sOuputFileName);
        return 1;
    }

    fprintf(fFileOutput,
            "@Echo Off\n"
            "SetLocal EnableExtensions\n");

    /* Print list of files for rebuild */
    for (i = 1; i < argc; i++) {
        sArg = argv[i];
        if (strnicmp(sArg, "/", 1) != 0) {
            sPathFile = sArg;
            sFileName = getFileName(sPathFile);
            buffer = file2buffer(sPathFile);
            bIsCabinet = isCabinet(&buffer);
            free(buffer.data);

            if (bIsCabinet) {
                fprintf(fFileOutput,
                        "Call :Rebuild \"%s\" 1\n", sPathFile);
            } else {
                fprintf(fFileOutput,
                        "Call :Rebuild \"%s\"\n", sPathFile);
            }

        }
        if (i == argc - 1) {
            fprintf(fFileOutput,
                    "Goto :Eof\n\n");
        }
    }

    /* Print resource data */
    for (i = 1; i < argc; i++) {
        sArg = argv[i];
        if (strnicmp(sArg, "/", 1) != 0) {
            sPathFile = sArg;
            sFileName = getFileName(sPathFile);
            buffer = file2buffer(sPathFile);

            if (NULL == buffer.data) {
                printf("Error reading the data of source: \"%s\"\n", sPathFile);
                return 1;
            }

            bhx(buffer, sFileName, bHexEncode, fFileOutput);
            free(buffer.data);
        }
    }

    PrintRebuildFunction(fFileOutput);
    fclose(fFileOutput);

    printf("\"%s\" generated.\n", sOuputFileName);

    return 0;

}

int bhx(struct BUFFER buffer, char *sfileName, BOOL bHexEncode, FILE *fFileOutput) {

    long i;
    long last_offset;
    UINT num;
    UINT divisor;
    int cnt;
    int chrs_printed;

    if (buffer.data == NULL) {
        return 1;
    }

    fprintf(fFileOutput, ":+res:b%d:%ld:%s:\n", (bHexEncode) ? 16 : 85, buffer.size, sfileName);

    last_offset = buffer.size - 1;
    chrs_printed = 0;

    if (bHexEncode) {

        for (i = 0; i <= last_offset; i++) {
            fprintf(fFileOutput, "%02x", buffer.data[i]);
            chrs_printed += 2;
            if ((64 == chrs_printed) || (i == last_offset)) {
                fprintf(fFileOutput, "\n");
                chrs_printed = 0;
            }
        }
    } else {

        cnt = 0;
        num = 0;
        for (i = 0; i <= last_offset; i++) {

            num = (num * 256) + buffer.data[i];
            cnt++;

            if (i == last_offset) {
                while (cnt < 4) { //padding
                    num = (num * 256);
                    cnt++;
                }
            }

            if (4 == cnt) {

                divisor = 85 * 85 * 85 * 85;
                while (divisor) {
                    fprintf(fFileOutput, "%c",
                            map_enc85[num / divisor % 85]);
                    divisor /= 85;
                }
                chrs_printed += 5;

                cnt = 0;
                num = 0;
            }

            if ((60 == chrs_printed) || (i == last_offset)) {
                chrs_printed = 0;
                fprintf(fFileOutput, "\n");
            }

        }

    }

    fprintf(fFileOutput, ":+res:b%d:%ld:%s:\n", (bHexEncode) ? 16 : 85, buffer.size, sfileName);
    fprintf(fFileOutput, "\n");
    return 0;

}

int PrintRebuildFunction(FILE * fileOutput) {

    int result;

    result = fprintf(fileOutput,
            ":Rebuild\n"
            "Rem Generated using BHX " bhx_version "\n"
            "SetLocal EnableExtensions EnableDelayedExpansion\n"
            "Set \"bin=%%~1\"\n"
            "Set \"expandCabinet=%%~2\"\n"
            "For %%%%# In (\n"
            "\"!bin!\" \"!bin!.da\" \"!bin!.tmp\"\n"
            ") Do If Exist \"%%%%#\" (Del /A /F /Q \"%%%%#\" >Nul 2>&1\n"
            "If ErrorLevel 1 Exit /B 1 )\n"
            "Set \"lbl=:+res:b[0-9]*:[0-9]*:!bin!:\"\n"
            "Set \"fsrc=%%~f0\"\n"
            "Findstr /I /B /N \"!lbl!\" \"!fsrc!\" >\"!bin!.tmp\"\n"
            "Set \"inioff=\"\n"
            "Set \"endoff=\"\n"
            "For /F \"usebackq tokens=1,3,4 delims=:\" %%%%a in (\"!bin!.tmp\"\n"
            ") Do If Not Defined inioff (\n"
            "Set \"inioff=%%%%~a\"\n"
            "Set \"base=%%%%~b\"\n"
            "Set /A \"size=%%%%~c\"\n"
            ") Else Set \"endoff=%%%%~a\"\n"
            "Set \".=ado=\"adodb.stream\"\"\n"
            "Set \".=!.! :set a=createobject(ado) :a.type=1 :a.open\"\n"
            "Set \".=!.! :set u=createobject(ado) :u.type=2 :u.open\"\n"
            "Set \".=!.! :set fs=createobject(\"scripting.filesystemobject\")\"\n"
            "Set \".=!.! :set s=fs.opentextfile(\"!fsrc!\",1,0,0)\"\n"
            "Set \".=!.! :e=\"0123456789abcdefghijklmnopqrstuvwxyzABCDEF\"\n"
            "Set \".=!.!GHIJKLMNOPQRSTUVWXYZ.-:+=^^`/*?&<>()[]{}~,$#\"\n"
            "Set \".=!.!\" :max=!size! :wri=0 :n=array(0,0,0,0,0)\"\n"
            "Set \".=!.! :for i=1 to !inioff! step 1 :s.readline :next\"\n"
            "Set \".=!.! :do while i<!endoff! :d=replace(s.readline,\" \",\"\")\"\n"
            "If /I \"!base!\"==\"b85\" (\n"
            "Set \".=!.! :for j=1 to len(d) step 5 :num85=mid(d,j,5)\"\n"
            "Set \".=!.! :v=0 :for k=1 to len(num85) step 1\"\n"
            "Set \".=!.! :v=v*85+instr(1,e,mid(num85,k,1))-1 :next\"\n"
            "Set \".=!.! :n(1)=Fix(v/16777216) :v=v-n(1)*16777216\"\n"
            "Set \".=!.! :n(2)=Fix(v/65536) :v=v-n(2)*65536\"\n"
            "Set \".=!.! :n(3)=Fix(v/256) :n(4)=v-n(3)*256\"\n"
            "Set \".=!.! :for m=1 to 4 step 1 :if (wri < max) then\"\n"
            "Set \".=!.! :u.writetext chrb(n(m)) :wri=wri+1 :end if :next\"\n"
            ") Else (Set \".=!.! :for j=1 to len(d) step 2\"\n"
            "Set \".=!.! :u.writetext chrb(\"^&h\"&mid(d,j,2))\" )\n"
            "Set \".=!.! :next :i=i+1 :loop\"\n"
            "Set \".=!.! :u.position=2 :u.copyto a :u.close :set u=nothing\"\n"
            "Set \".=!.! :a.savetofile \"!bin!\",2 :a.close :set a=nothing\"\n"
            "Set \".=!.! :s.close :set s=nothing :set fs=nothing\"\n"
            "Echo !.!>\"!bin!.da\"\n"
            "Set \"ret=1\"\n"
            "Cscript.exe /B /E:vbs \"!bin!.da\" >Nul\n"
            "For %%%%# In (\"!bin!\") Do If \"%%%%~z#\"==\"!size!\" Set \"ret=0\"\n"
            "If \"!expandCabinet!\"==\"1\" (\n"
            "If \"0\"==\"!ret!\" Expand.exe -r \"!bin!\" -F:* . >Nul\n"
            "If ErrorLevel 1 Set \"ret=1\"\n"
            "Del /A /F \"!bin!\" \"!bin!.da\" \"!bin!.tmp\" >Nul\n"
            ") Else (\n"
            "If \"1\"==\"!ret!\" If Exist \"!bin!\" Del /A /F \"!bin!\" >Nul\n"
            "Del /A /F \"!bin!.da\" \"!bin!.tmp\" >Nul\n"
            ")\n"
            "If \"1\"==\"!ret!\" Echo Rebuild failed: !bin!\n"
            "Exit /B !ret!\n");

    return result;
}

char *getFileName(char *filePath) {
    char *fileName = filePath;
    char *pi = filePath;
    while (*pi) {
        if (('\\' == *pi) || ('/' == *pi)) {
            fileName = pi + 1;
        }
        ++pi;
    }
    return fileName;
}

struct BUFFER file2buffer(char *fileName) {
    FILE *file;
    struct BUFFER buffer;

    buffer.data = NULL;
    buffer.size = 0;

    file = fopen(fileName, "rb");

    if (NULL != file) {
        /* Set position to end of file */
        fseek(file, 0L, SEEK_END);

        /* Set BUFFER.size to size of the file */
        buffer.size = ftell(file);

        /* Set position to beginning of file */
        fseek(file, 0L, SEEK_SET);

        /* Add 1 for allow null terminator */
        buffer.data = (BYTE *) malloc(buffer.size + 1);

        if (NULL != buffer.data) {
            fread(buffer.data, 1, buffer.size, file);
            if (ferror(file)) {
                free(buffer.data);
                buffer.data = NULL;
                buffer.size = 0;
                printf("Error: Reading the file.\n");

            } else {
                buffer.data[buffer.size] = '\0'; /* Prevent buffer overflow */
            }
        } else {
            buffer.size = 0;
            printf("Error: Out of memory.\n");
        }
        fclose(file);
    }
    return buffer;
}

BOOL isCabinet(struct BUFFER *buffer) {

    DWORD MINIMAL_CAB_SIZE = 62;
    int buffer_have_cabinet_header;
    char CABINET_HEADER[] = {'M', 'S', 'C', 'F', '\0', '\0', '\0', '\0'};
    int cabinet_header_size = (int) sizeof (CABINET_HEADER);
    WORD i;

    if (buffer->size < MINIMAL_CAB_SIZE) {
        return FALSE;
    }

    for (i = 0; i < cabinet_header_size; i++) {
        buffer_have_cabinet_header = CABINET_HEADER[i] == buffer->data[i];
        if (!buffer_have_cabinet_header) {
            return FALSE;
        }
    }

    return TRUE;
}
