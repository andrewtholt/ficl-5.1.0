/*
 ** stub main for testing Ficl
 ** $Id: main.c,v 1.2 2010/09/10 09:01:28 asau Exp $
 */
/*
 ** Copyright (c) 1997-2001 John Sadler (john_sadler@alum.mit.edu)
 ** All rights reserved.
 **
 ** Get the latest Ficl release at http://ficl.sourceforge.net
 **
 ** I am interested in hearing from anyone who uses Ficl. If you have
 ** a problem, a success story, a defect, an enhancement request, or
 ** if you would like to contribute to the Ficl release, please
 ** contact me by email at the address above.
 **
 ** L I C E N S E  and  D I S C L A I M E R
 ** 
 ** Redistribution and use in source and binary forms, with or without
 ** modification, are permitted provided that the following conditions
 ** are met:
 ** 1. Redistributions of source code must retain the above copyright
 **    notice, this list of conditions and the following disclaimer.
 ** 2. Redistributions in binary form must reproduce the above copyright
 **    notice, this list of conditions and the following disclaimer in the
 **    documentation and/or other materials provided with the distribution.
 **
 ** THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 ** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 ** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ** ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 ** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 ** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 ** OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 ** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 ** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 ** OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 ** SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "ficl.h"

#include <termios.h>
static int      ttyfd = 0;   /* STDIN_FILENO is 0 by default */

char *strsave(char *);
static void ficlDollarPrimitiveLoad(ficlVm *);
static void ficlPrimitiveLoad(ficlVm *);
void ficlSystemCompileMain(ficlSystem *);
static void athGetkey(ficlVm *);
static void athQkey(ficlVm *);
int tty_reset(void);

struct termios orig_termios; /* Terminal IO Structure */
char *loadPath;
#define BUFFER_SIZE 256
// char prompt[32];

void usage() {
    printf("\nUsage: ficl <options> where options are: \n\n");
    printf("\t-c <cmd>\tExecute the command and exit.\n");
    printf("\t-f <file>\tLoad this file at startup\n");
    printf("\t-h|?\t\tThis help.\n");
    printf("\t-q\t\tSupress startup messages.");

    printf("\n");

    printf("NOTES:\n");
    printf("\t-d is used with -f in the case where the program will be run as a\n");
    printf("\tbackground task, or started by a system script.\n\n");

    printf("\tThe environment variable FICL_PATH controls the locations searched for\n");
    printf("\tfiles.  It is a : seperated list of directories, e.g\n\n");

    printf("\t\t/usr/local/lib/ficl:/home/fred/ficl/lib:.\n\n");


    printf("\n\tFor Example;\n");
    printf("\n\t\texport FICL_PATH=\"/usr/local/lib/ficl:.\"\n\n");

    printf("\tTo display the options that were selected at build time, enter:\n\n");

    printf("\t\t.features\n\n");

}

extern int verbose;

int main(int argc, char **argv) {
    int returnValue = 0;
    char buffer[256];
    ficlVm *vm;
    ficlSystem *system;

    extern char prompt[];

    int i=0;
    int ch;
    extern char *optarg;

    char *fileName=(char *)NULL;
//    char *loadPath=(char *)NULL;
    char *cmd=(char *)NULL;

    strcpy(prompt, FICL_PROMPT);
    verbose=-1; // Default is to be talkative.

    i = tcgetattr( 0, &orig_termios); 

    loadPath = getenv("FICL_PATH");

    while ((ch = getopt(argc,argv, "c:qh?df:sV")) != -1) {
        switch(ch) {
            case 'c':
                cmd=strsave(optarg);
                printf("%s\n",cmd);
                break;
            case 'f':
                fileName=strsave(optarg);
                break;
            case 'h':
            case '?':
                usage();
                exit(0);
                break;
            case 'q':
                verbose=0;
                break;
        }
    }

    system = ficlSystemCreate(NULL);
    ficlSystemCompileExtras(system);
    ficlSystemCompileMain(system);
    vm = ficlSystemCreateVm(system);

    if(verbose !=0) {
        returnValue = ficlVmEvaluate(vm, ".ver .( " __DATE__ " ) cr quit");
        // returnValue = ficlVmEvaluate(vm, ".ver cr quit");
    }

    /*
     ** load files specified on command-line
     */

    if( cmd != (char *)NULL ) {
        returnValue = ficlVmEvaluate(vm, cmd);
        exit(returnValue);
    }

    if( fileName != (char *)NULL ) {
        if( verbose == 0) {
            sprintf(buffer, "load %s\n cr", fileName );
        } else {
            sprintf(buffer, ".( loading %s ) cr load %s\n cr", fileName, fileName );
        }
        returnValue = ficlVmEvaluate(vm, buffer);
    }

    while (returnValue != FICL_VM_STATUS_USER_EXIT) {
        //        fputs(FICL_PROMPT, stdout);
        fputs(prompt, stdout);
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
        returnValue = ficlVmEvaluate(vm, buffer);
    }

    ficlSystemDestroy(system);
    return 0;
}

#ifndef EMBEDDED
/* put terminal in raw mode - see termio(7I) for modes */

void tty_raw(void) {
    struct termios  raw;

    extern struct termios orig_termios; /* TERMinal I/O Structure */

    raw = orig_termios; /* copy original and then modify below */

    /*
     * input modes - clear indicated ones giving: no break, no CR to NL,
     * no parity check, no strip char, no start/stop output (sic) control
     */
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    /*
     * output modes - clear giving: no post processing such as NL to
     * CR+NL
     */
    raw.c_oflag &= ~(OPOST);

    /* control modes - set 8 bit chars */
    raw.c_cflag |= (CS8);

    /*
     * local modes - clear giving: echoing off, canonical off (no erase
     * with backspace, ^U,...),  no extended functions, no signal chars
     * (^Z,^C)
     */
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

    /*
     * control chars - set return condition: min number of bytes and
     * timer
     */
    //    raw.c_cc[VMIN] = 5;
    //    raw.c_cc[VTIME] = 8;    /* after 5 bytes or .8 seconds after first byte seen      */
    //    raw.c_cc[VMIN] = 0;
    //    raw.c_cc[VTIME] = 0;    /* immediate - anything       */
    //    raw.c_cc[VMIN] = 2;
    //    raw.c_cc[VTIME] = 0;    /* after two bytes, no timer  */
    //    raw.c_cc[VMIN] = 0;
    //    raw.c_cc[VTIME] = 8;    /* after a byte or .8 seconds */

    /* put terminal in raw mode after flushing */
    if (tcsetattr(ttyfd, TCSAFLUSH, &raw) < 0)
        fatal("can't set raw mode");
}
/*
 ** Ficl add-in to load a text file and execute it...
 ** Cheesy, but illustrative.
 ** Line oriented... filename is newline (or NULL) delimited.
 ** Example:
 **    load test.f
 */

char           *pathToFile(char *fname) {
    int             i;
    int             fd;
    extern char    *loadPath;
    char            path[255];
    char            scr[255];
    char           *scratch;
    char           *tok;
    char           *dirs[32];

    if ((loadPath == (char *) NULL) || (*fname == '/') || (*fname == '.'))
        return (fname);

    strcpy(path, loadPath);

    tok = (char *) strtok(path, ":");
    i = 0;

    for (; tok != (char *) NULL;) {
        strcpy(scr, tok);
        strcat(scr, "/");
        strcat(scr, fname);

        scratch = strsave(scr);

        fd = open(scratch, O_RDONLY);

        if (fd >= 0) {
            close(fd);
            return (scratch);
        }
        tok = (char *) strtok(NULL, ":");
    }
    return (NULL);
}

static void ficlDollarPrimitiveLoad(ficlVm * vm) {
    char            buffer[BUFFER_SIZE];
    char            filename[BUFFER_SIZE];
    char            fullName[255];

    char           *scratch;
    ficlCountedString *counted = (ficlCountedString *) filename;
    int             line = 0;
    FILE           *f;
    int             result = 0;
    ficlCell        oldSourceId;
    ficlString      s;
    int             nameLen;
    char           *name;
    char *ptr=(char *)NULL;

    nameLen = ficlStackPopInteger(vm->dataStack);
    ptr=ficlStackPopPointer(vm->dataStack);
    name=strtok(ptr," ");
    name[nameLen] = '\0';

    scratch = pathToFile(name);

    if (scratch == (char *) NULL) {
        sprintf(buffer, "File not found :%s", name);
        ficlVmTextOut(vm, buffer);
        ficlVmTextOut(vm, FICL_COUNTED_STRING_GET_POINTER(*counted));
        ficlVmTextOut(vm, "\n");
        ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
    } else {
        strcpy(fullName, scratch);
    }
    /*
     ** get the file's size and make sure it exists
     */
    f = fopen(fullName, "r");
    if (!f) {
        sprintf(buffer, "Unable to open file %s", name);
        ficlVmTextOut(vm, buffer);
        ficlVmTextOut(vm, FICL_COUNTED_STRING_GET_POINTER(*counted));
        ficlVmTextOut(vm, "\n");
        ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
    }
    oldSourceId = vm->sourceId;
    vm->sourceId.p = (void *) f;

    /* feed each line to ficlExec */
    while (fgets(buffer, BUFFER_SIZE, f)) {
        int             length = strlen(buffer) - 1;

        line++;
        if (length <= 0)
            continue;

        if (buffer[length] == '\n')
            buffer[length--] = '\0';

        FICL_STRING_SET_POINTER(s, buffer);
        FICL_STRING_SET_LENGTH(s, length + 1);
        result = ficlVmExecuteString(vm, s);
        /* handle "bye" in loaded files. --lch */
        switch (result) {
            case FICL_VM_STATUS_OUT_OF_TEXT:
                break;
            case FICL_VM_STATUS_USER_EXIT:
                exit(0);
                break;

            default:
                vm->sourceId = oldSourceId;
                fclose(f);
                ficlVmThrowError(vm, "Error loading file <%s> line %d", FICL_COUNTED_STRING_GET_POINTER(*counted), line);
                break;
        }
    }
    /*
     ** Pass an empty line with SOURCE-ID == -1 to flush
     ** any pending REFILLs (as required by FILE wordset)
     */
    vm->sourceId.i = -1;
    FICL_STRING_SET_FROM_CSTRING(s, "");
    ficlVmExecuteString(vm, s);

    vm->sourceId = oldSourceId;
    fclose(f);

    /* handle "bye" in loaded files. --lch */
    if (result == FICL_VM_STATUS_USER_EXIT)
        ficlVmThrow(vm, FICL_VM_STATUS_USER_EXIT);
    return;
}

static void ficlPrimitiveLoad(ficlVm * vm) {
    char            buffer[BUFFER_SIZE];
    char            filename[BUFFER_SIZE];
    char            scratch[255];
    char            tmp[255];

    extern char    *loadPath;
    char           *name;
    char           *tok;
    char           *dirs[32];

    int             i = 0;
    int             fd;


    ficlCountedString *counted = (ficlCountedString *) filename;
    ficlVmGetString(vm, counted, '\n');

    if (FICL_COUNTED_STRING_GET_LENGTH(*counted) <= 0) {
        ficlVmTextOut(vm, "Warning (load): nothing happened\n");
        return;
    }
    name = FICL_COUNTED_STRING_GET_POINTER(*counted);


    ficlStackPushPointer(vm->dataStack, name);
    ficlStackPushInteger(vm->dataStack, FICL_COUNTED_STRING_GET_LENGTH(*counted));
    ficlDollarPrimitiveLoad(vm);
}

static void ficlDollarPrimitiveLoadDir(ficlVm * vm) {
    char           *dirName, *fileName;
    char            buffer[255];

    int             dirLen, fileLen;

    fileLen = ficlStackPopInteger(vm->dataStack);
    fileName = ficlStackPopPointer(vm->dataStack);
    fileName[fileLen] = '\0';

    dirLen = ficlStackPopInteger(vm->dataStack);
    dirName = ficlStackPopPointer(vm->dataStack);
    dirName[dirLen] = '\0';

    sprintf(buffer, "%s/%s", dirName, fileName);
    ficlStackPushPointer(vm->dataStack, buffer);
    ficlStackPushInteger(vm->dataStack, strlen(buffer));
    ficlDollarPrimitiveLoad(vm);
}

#define EMPTY '\0'
static char cbuf = EMPTY;
static void athGetkey(ficlVm * vm) {

    int i;
    extern struct termios orig_termios; /* TERMinal I/O Structure */

    i = tcgetattr( ttyfd, &orig_termios);

    if( cbuf != EMPTY ) {
        ficlStackPushInteger(vm->dataStack, cbuf);
        cbuf = EMPTY ;
    } else {

        tty_raw();
        nonblock(0);
        i=fgetc(stdin);
        nonblock(1);
        tty_reset();
        ficlStackPushInteger(vm->dataStack, i);

    }
}


static void athQkey(ficlVm * vm) {

    tty_raw();
    nonblock(0);

    if(kbhit() !=0 )
    {
        cbuf=fgetc(stdin);
        ficlStackPushInteger(vm->dataStack, -1);
    } else {
        cbuf = EMPTY;
        ficlStackPushInteger(vm->dataStack,0);
    }
    tty_reset();
    nonblock(1);    

}

int tty_reset(void) {
    extern struct termios orig_termios; /* TERMinal I/O Structure */

    /* flush and reset */
    if (tcsetattr(ttyfd, TCSAFLUSH, &orig_termios) < 0) {
        return -1;
    }
    return 0;
}

void ficlSystemCompileMain(ficlSystem * system) {
    ficlDictionary *dictionary = ficlSystemGetDictionary(system);

    ficlDictionarySetPrimitive(dictionary, (char *)"load", ficlPrimitiveLoad, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"$load", ficlDollarPrimitiveLoad, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"key", athGetkey, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"?key", athQkey, FICL_WORD_DEFAULT);
}
#endif
