#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#ifndef EMBEDDED
#include <termios.h>
#include <sys/select.h>
#endif

#ifdef REDIS
#include "hiredis.h"
#endif 

static int      ttyfd = 0;   /* STDIN_FILENO is 0 by default */

#ifdef SYSV_IPC
#include <sys/ipc.h>
#include <sys/shm.h>
#endif

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "ficl.h"

#ifdef LIBMODBUS
#include <modbus/modbus.h>
#endif

#ifdef SPREAD
#warning "Including SPREAD header"
#include <sp.h>
#endif

#ifdef SOCKET
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

// #ifndef ARM
#ifndef EMBEDDED
#include <dlfcn.h>
#include <sys/utsname.h>
#include <termios.h>
#endif

#ifdef SYSV_IPC
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED) || defined(MAC) || defined(SOLARIS)
#warning "Including sys/sem.h"
#include <sys/sem.h>
/* union semun is defined by including <sys/sem.h> */
#endif
#endif

#if (defined(LINUX) && defined(GPIO))
#include <linux/gpio.h>
#endif

#if (defined(LINUX))
#include <sys/mman.h>
static int systemTick=0;
#endif

#if defined(SOLARIS) 
#warning "Solaris"
union semun {
    int             val;
    struct semid_ds *buf;
    ushort_t       *array;
} arg;
#endif

#define MSGSIZE 255
#if defined(SOLARIS) || defined(COBALT) || defined(MAC)
struct msgbuf {
    long            mtype;
    char            mtext[MSGSIZE];
};
#endif

#if (!defined(MAC) && !defined(SOLARIS)) && defined(SYSV_IPC) && !defined(COBALT)
#warning "Including sys/msg.h"
#include <sys/msg.h>
#define MSGSIZE 255

#if defined(SYSV_IPC ) 
/* #if !defined(SOLARIS) && !defined(CYC_TS) */
#warning "Defining msgbuf"

struct msgbuf {
    long            mtype;
    char            mtext[MSGSIZE];
};
/* #endif */
#endif

#warning "Defining seminfo"
/* according to X/OPEN we have to define it ourselves */
union semun {
    int             val;	/* value for SETVAL */
    struct semid_ds *buf;	/* buffer for IPC_STAT, IPC_SET */
    unsigned short *array;	/* array for GETALL, SETALL */
    /* Linux specific part: */
    struct seminfo *__buf;	/* buffer for IPC_INFO */
};
#endif

/*
   struct nlist *install(struct database *,char *, char *);
   */

char           *strsave(char *);
extern int      errno;
#ifndef FICL_ANSI

void fatal(char *message) {
    fprintf(stderr, "fatal error: %s\n", message);
    exit(1);
}


#ifndef EMBEDDED
/* put terminal in raw mode - see termio(7I) for modes */
#ifdef FRED
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
 * reset tty - useful also for restoring the terminal when this process
 * wishes to temporarily relinquish the tty
 */

int tty_reset(void) {
    extern struct termios orig_termios; /* TERMinal I/O Structure */

    /* flush and reset */
    if (tcsetattr(ttyfd, TCSAFLUSH, &orig_termios) < 0) {
        return -1;
    }
    return 0;
}
#endif // fred
#endif

int kbhit() {
    struct timeval  tv;
    fd_set          fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    //STDIN_FILENO is 0
    select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

#define NB_DISABLE 1
#define NB_ENABLE 0

#ifndef EMBEDDED
void nonblock(int state) {
    struct termios ttystate;

    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);

    if (state==NB_ENABLE)     {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    } else if (state==NB_DISABLE) {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

}
#endif

char keystroke(int t) {
    static char     buf[10];
    static int      total, next;
    //    struct sgttyb   ns;

    if (next >= total)     {
        switch (total = read(0, buf, sizeof(buf)))         {
            case -1:
                fatal("System call failure: read\n");
                break;
            case 0:
                fatal("Mysterious EOF\n");
                break;
            default:
                next = 0;
                break;
        }
    }
    return (buf[next++]);
}
// #define EMPTY '\0'
// static char cbuf = EMPTY;

#ifdef FRED
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
#endif

static void athStdoutFlush(ficlVm * vm) {
    fflush( (FILE *)NULL );
}

/*
 ** Ficl interface to _getcwd (Win32)
 ** Prints the current working directory using the VM's
 ** textOut method...
 */
#ifndef EMBEDDED
static void ficlPrimitiveGetCwd(ficlVm * vm) {
    char           *directory;

    directory = (char *) getcwd((char *) NULL, 80);
    ficlVmTextOut(vm, directory);
    ficlVmTextOut(vm, "\n");
    free(directory);
    return;
}

/*
 ** Ficl interface to _chdir (Win32)
 ** Gets a newline (or NULL) delimited string from the input
 ** and feeds it to the Win32 chdir function...
 ** Example:
 **    cd c:\tmp
 */
static void ficlPrimitiveChDir(ficlVm * vm) {
    ficlCountedString *counted = (ficlCountedString *) vm->pad;
    ficlVmGetString(vm, counted, '\n');

    if (counted->length > 0) {
        int             err = chdir(counted->text);
        if (err) {
            ficlVmTextOut(vm, "Error: path not found\n");
            ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
        }
    } else {
        ficlVmTextOut(vm, "Warning (chdir): nothing happened\n");
    }
    return;
}
#endif


static void ficlPrimitiveClock(ficlVm * vm) {
    clock_t         now = clock();
    ficlStackPushUnsigned(vm->dataStack, (ficlUnsigned) now);
    return;
}

#endif				/* FICL_ANSI */

/*
 ** Ficl interface to system (ANSI)
 ** Gets a newline (or NULL) delimited string from the input
 ** and feeds it to the ANSI system function...
 ** Example:
 **    system del *.*
 **    \ ouch!
 */
static void ficlPrimitiveSystem(ficlVm * vm) {
    ficlCountedString *counted = (ficlCountedString *) vm->pad;

    ficlVmGetString(vm, counted, '\n');
    if (FICL_COUNTED_STRING_GET_LENGTH(*counted) > 0)
    {
        int             returnValue = system(FICL_COUNTED_STRING_GET_POINTER(*counted));
        if (returnValue)
        {
            sprintf(vm->pad, "System call returned %d\n", returnValue);
            ficlVmTextOut(vm, vm->pad);
            ficlVmThrow(vm, FICL_VM_STATUS_QUIT);
        }
    } else
    {
        ficlVmTextOut(vm, "Warning (system): nothing happened\n");
    }
    return;
}

static void athFeatures(ficlVm *vm) {
#ifdef MAC
    printf ("    MAC\n");
#else
    printf ("NOT MAC\n");
#endif

#ifdef COBALT
    printf ("    COBALT\n");
#else
    printf ("NOT COBALT\n");
#endif

#ifdef LINUX
    printf ("    LINUX\n");
#else
    printf ("NOT LINUX\n");
#endif

#ifdef UCLINUX
    printf ("    UCLINUX\n");
#else
    printf ("NOT UCLINUX\n");
#endif

#ifdef SOLARIS
    printf("    SOLARIS\n");
#else
    printf("NOT SOLARIS\n");
#endif

#ifdef MTHREAD
    printf("    MTHREAD\n");
#else
    printf("NOT MTHREAD\n");
#endif


#ifdef QNX
    printf("    QNX\n");
#else
    printf("NOT QNX\n");
#endif
    printf("\n");

#ifdef FICL_ANSI 
    printf("    FICL_ANSI\n");
#else
    printf("NOT FICL_ANSI\n");
#endif

#ifdef FICL_WANT_MINIMAL
    printf("    FICL_WANT_MINIMAL\n");
#else
    printf("NOT FICL_WANT_MINIMAL\n");
#endif

#ifdef SERIAL
    printf ("    SERIAL\n");
#else
    printf ("NOT SERIAL\n");
#endif

#ifdef REDIS
    printf ("    REDIS\n");
#else
    printf ("NOT REDIS\n");
#endif

#ifdef MODBUS
    printf ("    MODBUS\n");
#else    
    printf ("NOT MODBUS\n");
#endif

#ifdef LIBMODBUS
    printf("    LIBMODBUS\n");
#else
    printf("NOT LIBMODBUS\n");
#endif

#ifdef SYSV_IPC
    printf ("    SYSV_IPC\n");
#else    
    printf ("NOT SYSV_IPC\n");    
#endif

#ifdef DYNLIB
    printf("    DYNLIB\n");
#else
    printf("NOT DYNLIB\n");
#endif


#ifdef SOCKET
    printf("    SOCKET\n");
#else
    printf("NOT SOCKET\n");
#endif

#ifdef I2C
    printf("    I2C\n");
#else
    printf("NOT I2C\n");
#endif

#if FICL_WANT_FILE
    printf("    WANT_FILE\n");
#else
    printf("NOT WANT_FILE\n");
#endif

#if FICL_WANT_FLOAT
    printf("    WANT_FLOAT\n");
#else
    printf("NOT WANT_FLOAT\n");
#endif

#if FICL_WANT_DEBUGGER
    printf("    WANT_DEBUGGER\n");
#else
    printf("NOT WANT_DEBUGGER\n");
#endif

#if SPREAD
    printf("    SPREAD\n");
#else
    printf("NOT SPREAD\n");
#endif
}

struct timeval now;

static void athNow(ficlVm *vm) {
    int status=0;

    status=gettimeofday(&now, NULL);
}

static void athElapsed(ficlVm *vm) {
    int status=0;
    int sec;
    int us;
    int msec;

    struct timeval here;

    status=gettimeofday(&here, NULL);

    sec = here.tv_sec - now.tv_sec;
    us  = here.tv_usec - now.tv_usec;

    msec = (sec * 1000) + ( us / 1000 );

    ficlStackPushInteger(vm->dataStack, msec);

}

static void athTime(ficlVm * vm) {
    time_t          t;

    t = time((time_t *) NULL);
    ficlStackPushInteger(vm->dataStack, t);
}

static void athDate(ficlVm * vm) {
    time_t          t;
    struct tm *d;

    t = (time_t)ficlStackPopInteger(vm->dataStack);
    d = localtime(&t);

    ficlStackPushInteger(vm->dataStack, (d->tm_year+1900));
    ficlStackPushInteger(vm->dataStack, (d->tm_mon+1));
    ficlStackPushInteger(vm->dataStack, d->tm_mday );
    ficlStackPushInteger(vm->dataStack, d->tm_sec );
    ficlStackPushInteger(vm->dataStack, d->tm_min );
    ficlStackPushInteger(vm->dataStack, d->tm_hour );
}

static void athPrimitiveDollarSystem(ficlVm * vm) {
    char           *cmd;
    int             len;
    int             status;

    len = ficlStackPopInteger(vm->dataStack);
    cmd = ficlStackPopPointer(vm->dataStack);

    if (len > 0)
        cmd[len] = '\0';

    status = system(cmd);

    ficlStackPushInteger(vm->dataStack, status);
}

#ifdef FRED
/*
 ** Ficl add-in to load a text file and execute it...
 ** Cheesy, but illustrative.
 ** Line oriented... filename is newline (or NULL) delimited.
 ** Example:
 **    load test.f
 */
#define BUFFER_SIZE 256

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
        } else {
            free(scratch);
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

#endif // FRED



/*
 ** Dump a tab delimited file that summarizes the contents of the
 ** dictionary hash table by hashcode...
 */
    static void
ficlPrimitiveSpewHash(ficlVm * vm)
{
    ficlHash       *hash = ficlVmGetDictionary(vm)->forthWordlist;
    ficlWord       *word;
    FILE           *f;
    unsigned        i;
    unsigned        hashSize = hash->size;

    if (!ficlVmGetWordToPad(vm))
        ficlVmThrow(vm, FICL_VM_STATUS_OUT_OF_TEXT);

    f = fopen(vm->pad, "w");
    if (!f)
    {
        ficlVmTextOut(vm, "unable to open file\n");
        return;
    }
    for (i = 0; i < hashSize; i++)
    {
        int             n = 0;

        word = hash->table[i];
        while (word)
        {
            n++;
            word = word->link;
        }

        fprintf(f, "%d\t%d", i, n);

        word = hash->table[i];
        while (word)
        {
            fprintf(f, "\t%s", word->name);
            word = word->link;
        }

        fprintf(f, "\n");
    }

    fclose(f);
    return;
}

char           *strsave(char *s) {
    char           *p;

    if ((p = (char *) malloc(strlen(s) + 1)) != NULL)
        strcpy(p, s);
    return (p);
}

#if FICL_WANT_STRING
#warning "String stack words...."
static void athStringPush(ficlVm *vm) {
    char *p,*n;
    int l;

    l= ficlStackPopInteger(vm->dataStack);
    p = ficlStackPopPointer(vm->dataStack);

    ficlStackPushPointer(vm->stringStack,(char *)strsave(p));

}

static void athStringPop(ficlVm *vm) {
    char *d,*n;
    int l;

    l= ficlStackPopInteger(vm->dataStack);
    d=ficlStackPopPointer(vm->dataStack);
    n=ficlStackPopPointer(vm->stringStack);

    strncpy(d,n,l);
    free(n);
    ficlStackPushInteger(vm->dataStack,strlen(d));

}

static void athStringJoin(ficlVm *vm)
{
    char *a,*b;
    char *new;
    int la,lb,ln;

    b=ficlStackPopPointer(vm->stringStack);
    a=ficlStackPopPointer(vm->stringStack);

    la=strlen(a);
    lb=strlen(b);
    ln=la+lb;

    new=(char *)malloc( ln+1); // space for null terminator

    strcpy(new,a);
    strcat(new,b);
    free(a);
    free(b);

    ficlStackPushPointer(vm->stringStack,new);
}

#endif

    static void
athSlashString(ficlVm * vm)
{
    char           *str;
    int             len, cut;

    cut = ficlStackPopInteger(vm->dataStack);
    len = ficlStackPopInteger(vm->dataStack);
    str = ficlStackPopPointer(vm->dataStack);

    len = len - cut;
    str = str + cut;

    ficlStackPushPointer(vm->dataStack, str);
    ficlStackPushInteger(vm->dataStack, len);

}

    static void
athMinusTrailing(ficlVm * vm)
{
    char           *str;
    int             len;
    int             i;

    len = ficlStackPopInteger(vm->dataStack);
    str = ficlStackPopPointer(vm->dataStack);

    for (i = (len - 1); i >= 0; i--)
    {
        if (str[i] > ' ')
            break;
        else
            len--;
    }
    ficlStackPushPointer(vm->dataStack, str);
    ficlStackPushInteger(vm->dataStack, len);
}

    static void
athTwoRot(ficlVm * vm)
{
    void           *stuff[6];
    void           *tmp0, *tmp1;
    int             i = 0;

    for (i = 0; i < 6; i++)
    {
        stuff[i] = (void *) ficlStackPopInteger(vm->dataStack);
    }

    for (i = 3; i >= 0; i--)
    {
        ficlStackPushPointer(vm->dataStack, stuff[i]);
    }
    ficlStackPushPointer(vm->dataStack, stuff[5]);
    ficlStackPushPointer(vm->dataStack, stuff[4]);
}

static void athStrsave(ficlVm * vm) {
    int             len;
    char           *str;
    char           *p;

    len = ficlStackPopInteger(vm->dataStack);
    str = ficlStackPopPointer(vm->dataStack);

    str[len] = '\0';
    p = (char *) strsave(str);

    ficlStackPushPointer(vm->dataStack, p);
    ficlStackPushInteger(vm->dataStack, len);
}

#ifdef DYNLIB
#define MAX_ARGS 5

// arg0 ... argn <res_count> <arg_count> <function ptr>
// 
static void athDlExec(ficlVm * vm) {
    int i;
    int argCount=0;
    int resCount=0;

    void *(*func)();
    void *args[MAX_ARGS];
    void *res;

    func = ficlStackPopPointer(vm->dataStack);
    argCount = ficlStackPopInteger(vm->dataStack);
    resCount = ficlStackPopInteger(vm->dataStack);

    for (i=0; i < argCount;i++) {
        args[i]  = ficlStackPopPointer(vm->dataStack);
    }

    if ( 0 == argCount && 0 == resCount ) {
        (void)(*func)();
    } else {
        if(argCount > 0 ) {
            switch(argCount) {
                case 1:
                    res=(*func)(args[0]);
                    break;
                case 2:
                    res=(*func)(args[1],args[0]);
                    break;
                case 3:
                    res=(*func)(args[2],args[1],args[0]);
                    break;
                case 4:
                    res=(*func)(args[3],args[2],args[1],args[0]);
                    break;
                case 5:
                    res=(*func)(args[4],args[3],args[2],args[1],args[0]);
                    break;
                default:
                    printf("Too many args\n");
                    break;
            }
            ficlStackPushPointer( vm->dataStack,res);
        } else {
            switch(argCount) {
                case 1:
                    (void)(*func)(args[0]);
                    break;
                case 2:
                    (void)(*func)(args[1],args[0]);
                    break;
                case 3:
                    (void)(*func)(args[2],args[1],args[0]);
                    break;
                case 4:
                    (void)(*func)(args[3],args[2],args[1],args[0]);
                    break;
                case 5:
                    (void)(*func)(args[4],args[3],args[2],args[1],args[0]);
                    break;
                default:
                    printf("Too many args\n");
                    break;
            }
        }
    }

}

static void athDlOpen(ficlVm * vm) {
    int libLen;
    char *lib;
    void *res;

    libLen = ficlStackPopInteger(vm->dataStack);
    lib = ficlStackPopPointer(vm->dataStack);

    res=(void *)dlopen(lib,RTLD_LAZY );

    if(!res) {
        printf("%s\n", dlerror());
    }

    ficlStackPushPointer( vm->dataStack,res);

    if( res == (void *)NULL ) {
        ficlStackPushInteger(vm->dataStack,-1);
    } else {
        ficlStackPushInteger(vm->dataStack,0);
    }
}

static void athDlClose(ficlVm * vm) {
    int res;
    void *h;

    h = ficlStackPopPointer( vm->dataStack);
    res = dlclose(h);
    ficlStackPushInteger(vm->dataStack,res);
}
/*
   name len handle -- symbol_ptr
   */
static void athDlSym(ficlVm * vm) {
    void *sym, *h;
    char *symbol;
    char *error;
    char flag=0;

    int symbolLen = 0;

    h = ficlStackPopPointer( vm->dataStack);
    symbolLen = ficlStackPopInteger( vm->dataStack);
    symbol = ficlStackPopPointer(vm->dataStack);
    symbol[symbolLen]=0x00;

    error = dlerror();
    symbol = dlsym(h,symbol);
    error = dlerror();

    if( error != (char *)NULL ) {
        fprintf(stderr,"%s\n",error);
        flag=-1;
    } else {
        flag=0;
    }

    ficlStackPushPointer( vm->dataStack,symbol);
    ficlStackPushInteger( vm->dataStack,flag);

}

    static void
athDlError(ficlVm *vm)
{
    perror(dlerror());
}
#endif

// Same as move except adds a 0 byte at the end.
// Traget area must be at least one byte bigger than len
//
// Stack: from to len --
static void athZmove(ficlVm *vm) {
    char *from, *to, *ret;
    int len;

    len = ficlStackPopInteger(vm->dataStack);
    to = ficlStackPopPointer(vm->dataStack);
    from = ficlStackPopPointer(vm->dataStack);

    ret = strncpy(to,from,len);
    *(to + len) = (char)NULL;

}
// 
// Add an end of line char (0x0a) to a string.
// Space must already exist for tis.
//
// Stack : addr len -- addr len+1
//
static void athAddCr(ficlVm *vm) {
    char *from;
    int len;

    len = ficlStackPopInteger(vm->dataStack);
    from = ficlStackPopPointer(vm->dataStack);

    *(from + len) = (char)0x0a;

    len++;

    ficlStackPushPointer( vm->dataStack,from);
    ficlStackPushInteger(vm->dataStack, len);

}

static void athSizeofInt(ficlVm *vm)
{
    ficlStackPushInteger(vm->dataStack, sizeof(int));
}

static void athSizeofChar(ficlVm *vm)
{
    ficlStackPushInteger(vm->dataStack, sizeof(char));
}

static void athSizeofCharPtr(ficlVm *vm)
{
    ficlStackPushInteger(vm->dataStack, sizeof(char *));
}

    static void
ficlPrimitiveBreak(ficlVm * vm)
{
    //	vm->state = vm->state;
    return;
}

static void athGetPid(ficlVm *vm)
{
    ficlStackPushInteger(vm->dataStack, (int) getpid() );
}

#define READ 0
#define WRITE 1

#ifndef  EMBEDDED
pid_t popen2(const char *command, int *infp, int *outfp)
{
    int p_stdin[2], p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
        return -1;

    pid = fork();

    if (pid < 0)
        return pid;
    else if (pid == 0)
    {
        close(p_stdin[WRITE]);
        dup2(p_stdin[READ], READ);
        close(p_stdout[READ]);
        dup2(p_stdout[WRITE], WRITE);

        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl");
        exit(1);
    }

    if (infp == NULL)
        close(p_stdin[WRITE]);
    else
        *infp = p_stdin[WRITE];

    if (outfp == NULL)
        close(p_stdout[READ]);
    else
        *outfp = p_stdout[READ];

    return pid;
}

/**
 * bidirectional popen() call
 *
 * @param rwepipe - int array of size three
 * @param exe - program to run
 * @param argv - argument list
 * @return pid or -1 on error
 *
 * The caller passes in an array of three integers (rwepipe), on successful
 * execution it can then write to element 0 (stdin of exe), and read from
 * element 1 (stdout) and 2 (stderr).
 */

static int popenRWE(int *rwepipe, const char *exe, const char *const argv[]) {
    int in[2];
    int out[2];
    int err[2];
    int pid;
    int rc;

    rc = pipe(in);
    if (rc<0)
        goto error_in;

    rc = pipe(out);
    if (rc<0)
        goto error_out;

    rc = pipe(err);
    if (rc<0)
        goto error_err;

    pid = fork();
    if (pid > 0) { // parent
        close(in[0]);
        close(out[1]);
        close(err[1]);
        rwepipe[0] = in[1];
        rwepipe[1] = out[0];
        rwepipe[2] = err[0];
        return pid;
    } else if (pid == 0) { // child
        close(in[1]);
        close(out[0]);
        close(err[0]);
        close(0);
        dup(in[0]);
        close(1);
        dup(out[1]);
        close(2);
        dup(err[1]);

        execvp(exe, (char**)argv);
        exit(1);
    } else {
        goto error_fork;
    }

    return pid;

error_fork:
    close(err[0]);
    close(err[1]);
error_err:
    close(out[0]);
    close(out[1]);
error_out:
    close(in[0]);
    close(in[1]);
error_in:
    return -1;
}

static int pcloseRWE(int pid, int *rwepipe)
{
    int rc, status;
    close(rwepipe[0]);
    close(rwepipe[1]);
    close(rwepipe[2]);
    rc = waitpid(pid, &status, 0);
    return status;
}

/*
 * Need to fix a number of BUGS in this
 * 1. Deal with popenRWE returning an error.
 * 2. Close needs to free the ficlFile descriptors.
 */
void athPopenRWE(ficlVm *vm) {
    int len;
    int rwepipe[3];
    int i=0;
    int ret;

    char *cmd;
    char *args[32];
    char *tmp;
    char *exe;
    ficlFile *pStdin;
    ficlFile *pStdout;
    ficlFile *pStderr;

    len = ficlStackPopInteger(vm->dataStack);
    cmd = ficlStackPopPointer(vm->dataStack);
    cmd[len] = '\0';

    exe=(char *)strtok(cmd," ");

    i=0;
    args[i++]=exe;

    while( (tmp=(char *)strtok(NULL," "))) {
        //        printf("%s\n",tmp);
        args[i++] = tmp;
    }
    args[i]=(char *)NULL;

    ret = access( exe , R_OK | X_OK );

    if( 0 == ret ) {

        pStdin = (ficlFile *)malloc(sizeof(struct ficlFile));
        pStdout= (ficlFile *)malloc(sizeof(struct ficlFile));
        pStderr= (ficlFile *)malloc(sizeof(struct ficlFile));

        /*
           exe=(char *)strtok(cmd," ");

           i=0;
           args[i++]=exe;

           while( (tmp=(char *)strtok(NULL," "))) {
        //        printf("%s\n",tmp);
        args[i++] = tmp;
        }
        args[i]=(char *)NULL;
        */

        ret=popenRWE(&rwepipe[0],cmd,(const char *)args);

        pStdin->fd = rwepipe[0];
        pStdin->f = fdopen(rwepipe[0],"w");
        strcpy( pStdin->filename, "Pipe:stdin");

        pStdout->fd=rwepipe[1];
        pStdout->f = fdopen(rwepipe[1],"r");
        strcpy( pStdout->filename, "Pipe:stdout");

        pStderr->fd=rwepipe[2];
        pStderr->f = fdopen(rwepipe[2],"r");
        strcpy( pStderr->filename, "Pipe:stderr");

        ficlStackPushPointer(vm->dataStack, pStderr);
        ficlStackPushPointer(vm->dataStack, pStdout);
        ficlStackPushPointer(vm->dataStack, pStdin);
        ficlStackPushInteger(vm->dataStack, ret); // pid
        ficlStackPushInteger(vm->dataStack, 0);   // Sucess
    } else {
        ficlStackPushInteger(vm->dataStack, errno); 
        ficlStackPushInteger(vm->dataStack, -1); // fail :(
    }
}

void athPcloseRWE(ficlVm *vm) 
{
    int rwepipe[3];
    int pid;
    int status;

    ficlFile *pStdin;
    ficlFile *pStdout;
    ficlFile *pStderr;

    pid     = ficlStackPopInteger( vm->dataStack );
    pStderr = (ficlFile *)ficlStackPopPointer( vm->dataStack);
    pStdout = (ficlFile *)ficlStackPopPointer( vm->dataStack);
    pStdin  = (ficlFile *)ficlStackPopPointer( vm->dataStack);

    rwepipe[0] = pStdin->fd;
    rwepipe[1] = pStdout->fd;
    rwepipe[2] = pStdin->fd;
    pcloseRWE(pid,&rwepipe[0]);

    free( pStdin );
    free( pStdout );
    free( pStderr );
}
#endif

static int lastSignal;
    void
signalHandler(int sig)
{
    lastSignal = sig;
}

void getLastSignal(ficlVm *vm)
{
    ficlStackPushInteger(vm->dataStack, lastSignal);
    lastSignal=0;
}

#ifndef EMBEDDED
struct sigaction act;

static void athSignal(ficlVm * vm) {
    int             sig;
    sig = ficlStackPopInteger(vm->dataStack);
    act.sa_handler = signalHandler;
    act.sa_flags = 0;
    sigaction(sig, &act, NULL);
}

static void athKill(ficlVm *vm) {
    pid_t pid;
    int sig;

    pid = (pid_t)ficlStackPopInteger(vm->dataStack);
    sig = ficlStackPopInteger(vm->dataStack);

    ficlStackPushInteger(vm->dataStack, kill(pid,sig) );
}


static void athSetAlarm(ficlVm * vm) {
    int             t;
    t = ficlStackPopInteger(vm->dataStack);
    alarm(t);

}
#endif

    static void
athPerror(ficlVm * vm)
{
    perror("ficl");
}

    static void
athClrErrno(ficlVm * vm)
{
    extern int      errno;
    errno = 0;
}

    static void
athGetenv(ficlVm * vm)
{
    char           *ptr;
    char           *env;
    char           *tmp;

    int             len;

    len = ficlStackPopInteger(vm->dataStack);
    env = (char *)ficlStackPopPointer(vm->dataStack);
    ptr = (char *)ficlStackPopPointer(vm->dataStack);

    env[len] = '\0';
    tmp = getenv(env);
    if (tmp)
    {
        strcpy(ptr, tmp);
        len = strlen(tmp);
    } else
        len = 0;

    ficlStackPushPointer(vm->dataStack, ptr);
    ficlStackPushInteger(vm->dataStack, len);

}

    static void
athGetErrno(ficlVm * vm)
{
    extern int      errno;
    ficlStackPushInteger(vm->dataStack, errno);
    errno = 0;
}
#ifdef SYSV_IPC

    static void
athSemTran(ficlVm * vm)
{
    int             key;
    int             sid;
    extern int      errno;

    key = ficlStackPopInteger(vm->dataStack);
    if (key < 0)
    {
        ficlStackPushInteger(vm->dataStack, -1);
    } else
    {
        sid = semget(key, 1, 0666 | IPC_CREAT);

        ficlStackPushInteger(vm->dataStack, sid);

    }
}

static void athRmSem(ficlVm * vm) {

    int             sid;
    int             res;
#ifdef COBALT
    union semun arg;
#endif

    sid = ficlStackPopInteger(vm->dataStack);
#ifdef COBALT
    res = semctl(sid, 0, IPC_RMID,arg);
#else
    res = semctl(sid, 0, IPC_RMID);
#endif
    if (res < 0) {
        ficlStackPushInteger(vm->dataStack, -1);
    } else {
        ficlStackPushInteger(vm->dataStack, 0);
    }


}
/*
   Returns 0 on error, true on success.
   */
int semcall(int sid, int op) {
    struct sembuf   sb;

    sb.sem_num = 0;
    sb.sem_op = op;
    /*
     * sb.sem_flg = IPC_NOWAIT;
     */
    sb.sem_flg = 0;

    if (semop(sid, &sb, 1) == -1) {
        perror("ficl: semcall ");
        return (-1);
    } else {
        return (0);
    }
}

    static void
athGetSemValue(ficlVm * vm)
{
    int             sid;
    int             res;
#ifdef COBALT
    union semun arg;
#endif

    sid = ficlStackPopInteger(vm->dataStack);
#ifdef COBALT
    res = semctl(sid, 0, GETVAL,arg);
#else
    res = semctl(sid, 0, GETVAL);
#endif
    ficlStackPushInteger(vm->dataStack, res);
}

/*
 * Stack: value semid -- flag
 */
    static void
athSetSemValue(ficlVm * vm)
{
    int             sid;
    int             res;
    int             val;
    union semun     arg;

    sid = ficlStackPopInteger(vm->dataStack);
    val = ficlStackPopInteger(vm->dataStack);
    arg.val = val;
    res = semctl(sid, 0, SETVAL, arg);
    if (!res)
        res = -1;
    else
        res = 0;

    ficlStackPushInteger(vm->dataStack, res);
}

    static void
athGetSem(ficlVm * vm)
{
    int             sid;

    sid = ficlStackPopInteger(vm->dataStack);


    ficlStackPushInteger(vm->dataStack,semcall(sid, -1) );
}

    static void
athRelSem(ficlVm * vm)
{
    int             sid;

    sid = ficlStackPopInteger(vm->dataStack);
    ficlStackPushInteger(vm->dataStack, semcall(sid, 1));
}
/*
 * Stack: key size flags -- shmid
 */
    static void
athShmGet(ficlVm * vm)
{
    int             shm_id;
    int             key;
    int             size;
    int             flags;

    flags = ficlStackPopInteger(vm->dataStack);
    size = ficlStackPopInteger(vm->dataStack);
    key = ficlStackPopInteger(vm->dataStack);
    shm_id = shmget(key, size, flags);
    ficlStackPushInteger(vm->dataStack, shm_id);
}

/*
 * Stack: shmid -- ptr
 */
    static void
athShmat(ficlVm * vm)
{
    void           *ptr;
    int             shm_id;

    shm_id = ficlStackPopInteger(vm->dataStack);
    ptr = shmat(shm_id, 0, SHM_RND);
    ficlStackPushPointer(vm->dataStack, ptr);
}

/*
 * Stack: ptr -- flag
 */
    static void
athShmdt(ficlVm * vm)
{
    void           *ptr;
    int             res;
    ptr = ficlStackPopPointer(vm->dataStack);
    res = shmdt(ptr);
    if (!res)
        ficlStackPushInteger(vm->dataStack, -1);
    else
        ficlStackPushInteger(vm->dataStack, 0);

}
/*
*/
static void athShmrm(ficlVm *vm)
{
    int shm_id;
    struct shmid_ds buf;
    int res;

    shm_id = ficlStackPopInteger(vm->dataStack);
    res = shmctl(shm_id,IPC_STAT,&buf);
    if ( res == 0 ) {
        res = shmctl(shm_id,IPC_RMID,&buf);
    }
    ficlStackPushInteger(vm->dataStack, res);
}

#endif


// #if !defined(MAC) && defined(SYSV_IPC)
#if defined(SYSV_IPC)
#define MAXOPEN 20

    static void
athOpenQueue(ficlVm * vm)
{
    static struct
    {
        long            key;
        int             qid;
    }               queues[MAXOPEN];
    int             i;
    int             avail = -1;
    int             qid;

    int             key;
    key = ficlStackPopInteger(vm->dataStack);

    //
    //Try to open the message Q first
    //
    if ((qid = msgget(key, 0666)) == -1)
    {
        if ((qid = msgget(key, 0666 | IPC_CREAT)) == -1)
        {
            ficlStackPushInteger(vm->dataStack, -1);
            return;
        }
    }
    queues[avail].key = key;
    queues[avail].qid = qid;
    ficlStackPushInteger(vm->dataStack, qid);
    ficlStackPushInteger(vm->dataStack, 0);
}

static void athRmQueue( ficlVm *vm)
{
    int msqid;

    msqid = ficlStackPopInteger(vm->dataStack);
    msgctl(msqid, IPC_RMID, NULL);
}
//
//Stack:ptr cnt qid-- flag
//
    static void
athMsgSend(ficlVm * vm)
{
    struct msgbuf   buf;
    int             nbytes;
    int             qid;
    char           *msg;
    int             status;

    qid = ficlStackPopInteger(vm->dataStack);
    nbytes = ficlStackPopInteger(vm->dataStack);
    msg = (char *)ficlStackPopPointer(vm->dataStack);

    buf.mtype = 1;
    strncpy(buf.mtext, msg, nbytes);

    //status = msgsnd(qid, &buf, nbytes - sizeof(buf.mtype), 0);
    status = msgsnd(qid, &buf, nbytes, 0);
    ficlStackPushInteger(vm->dataStack, status);
}
//
//Stack ptr qid-- ptr cnt flag
//
    static void
athMsgRecv(ficlVm * vm)
{
    struct msgbuf   buf;
    int             nbytes;
    int             qid;
    char           *msg;
    int             status;
    int flag = 0;

    flag = ficlStackPopInteger(vm->dataStack);
    qid = ficlStackPopInteger(vm->dataStack);
    msg = (char *)ficlStackPopPointer(vm->dataStack);

    status = msgrcv(qid, &buf, MSGSIZE, 1, flag);
    strncpy(msg, buf.mtext, MSGSIZE);

    if (status > 0)
    {
        ficlStackPushPointer(vm->dataStack, msg);
        ficlStackPushInteger(vm->dataStack, status);
        ficlStackPushInteger(vm->dataStack, 0);
    } else
    {
        ficlStackPushInteger(vm->dataStack, -1);
    }
}
#endif				/* // #define(MAC) */

/*
 * Stack: sep ptr cnt --- addrn lenn ...... addr0 len0 n
 */

    static void
athStrTok(ficlVm * vm)
{
    char           *ptr;
    char            s;
    char            sep[2];
    int             len;
    char           *t[255];
    int             i = 0;
    int             count = 0;

    char           *tok;

    len = ficlStackPopInteger(vm->dataStack);
    ptr = (char *)ficlStackPopPointer(vm->dataStack);
    s = ficlStackPopInteger(vm->dataStack);

    sep[0] = s;
    sep[1] = '\0';

    *(ptr + len) = '\0';

    count = 0;
    do
    {
        if (count == 0)
            tok = (char *) strtok(ptr, sep);
        else
            tok = (char *) strtok(NULL, sep);

        if (tok)
            t[count++] = tok;
    }
    while (tok != NULL);
    //	count;

    for (i = count - 1; i >= 0; i--)
    {
        /*
         * printf("i = %d\n",i); printf("t[%d] = %s \n",i,t[i]);
         */

        len = strlen(t[i]);
        ficlStackPushPointer(vm->dataStack, t[i]);
        ficlStackPushInteger(vm->dataStack, len);
    }
    ficlStackPushInteger(vm->dataStack, count);
}

static void athFiclFileDump(ficlVm *vm)
{
    ficlFile *a;

    a = (ficlFile *)ficlStackPopPointer(vm->dataStack);

    printf("FILE *: %x\n",(unsigned int)a->f);
    printf("Name  : %s\n",a->filename);
    printf("fd    : %d\n",a->fd);
}

static void athMs(ficlVm * vm) {
    int             ms;
    int i;
    int status;
#ifdef LINUX
    struct timespec tim,tim2;
#endif

    ms = ficlStackPopInteger(vm->dataStack);
    for (i=0;i < ms; i++) {
#ifdef LINUX
        usleep(1000);

        tim.tv_sec=0;
        tim.tv_nsec=( 1000 * 1000 * ms );
        (void) nanosleep(&tim,&tim2);
#else
        if ( usleep(1000) < 0)
            return;
#endif
    }
}


#ifdef SPREAD
#warning "... SPREAD"

static void athSPConnect(ficlVm * vm) {
    static mailbox  Mbox;
    char            User[80];
    char            Spread_name[80];
    char            buffer[255];
    char           *tmp, *tok;

    char            Private_group[MAX_GROUP_NAME];

    int             ret;
    int             len;
    int             i;

    int             service_type = 0;
    static char     mess[102400];
    char            sender[MAX_GROUP_NAME];
    char            target_groups[100][MAX_GROUP_NAME];
    int             num_groups;
    membership_info memb_info;
    int16           mess_type;
    int             endian_mismatch;

    len = ficlStackPopInteger(vm->dataStack);
    tmp = (char *) ficlStackPopPointer(vm->dataStack);
    tok = (char *) strtok(tmp, ":");
    (void) strcpy(User, tok);

    tok = (char *) strtok(NULL, ":");
    (void) strcpy(Spread_name, tok);


    ret = SP_connect(Spread_name, User, 0, 1, &Mbox, Private_group);

    if (ret < 0) {
        ficlStackPushInteger(vm->dataStack, -1);
        SP_error(ret);
    } else {
        SP_join(Mbox, "global");
        ficlStackPushInteger(vm->dataStack, Mbox);
        ficlStackPushInteger(vm->dataStack, 0);
    }
}   

static void athSPLeave(ficlVm * vm) {
    mailbox         Mbox;
    char           *group;
    int             len;
    int             ret;

    len = ficlStackPopInteger(vm->dataStack);
    group = (char *) ficlStackPopPointer(vm->dataStack);

    Mbox = ficlStackPopInteger(vm->dataStack);

    ret = SP_leave(Mbox, group);
    ficlStackPushInteger(vm->dataStack, ret);
}

static void athSPJoin(ficlVm * vm) {
    mailbox         Mbox;
    char           *tmp;
    int             len;
    char            group[80];
    char           *grp;
    int             grp_len;
    int             ret;
    int i=0;

    int             service_type = 0;
    static char     mess[102400];
    char            sender[MAX_GROUP_NAME];
    char            target_groups[100][MAX_GROUP_NAME];
    int             num_groups;
    membership_info memb_info;
    int16           mess_type;
    int             endian_mismatch;

    grp_len = ficlStackPopInteger(vm->dataStack);
    grp = (char *) ficlStackPopPointer(vm->dataStack);
    grp[grp_len] = '\0';

    /*
    //String is < user >:<host > so for example :
    //andrew:	4803
    // fred:		4803 @ host
    */

    Mbox = ficlStackPopInteger(vm->dataStack);

    /*
    //TODO Sun Jul 15 2007
    // When a group is joined capture the list of members.
    // On leave free the memory.
    // On send and recv use any membership messages to update list.
    */
    SP_join(Mbox, grp);
    ret = SP_receive(Mbox, &service_type, sender, 100, &num_groups, target_groups, &mess_type, &endian_mismatch, sizeof(mess), mess);

    if (ret < 0) {
        SP_error(ret);
    } else if (Is_regular_mess(service_type)) {
        printf("REGULAR MESSAGE\n");
    } else if (Is_membership_mess(service_type)) {
        ret = SP_get_memb_info(mess, service_type, &memb_info);

        if (ret < 0) {
            printf("BUG: membership message does not have valid body\n");
        }

        for (i = 0; i < num_groups; i++) {
            printf("Debug\t%s\n", &target_groups[i][0]);
        }
    }
}

static void athSPSend(ficlVm * vm) {
    mailbox         Mbox;
    char           *tmp;
    int             len;

    static char     mess[102400];
    char            sender[MAX_GROUP_NAME];
    char            target_groups[100][MAX_GROUP_NAME];
    int             num_groups;
    membership_info memb_info;
    int             service_type;
    int16           mess_type;
    int             endian_mismatch;
    int             i;
    int             ret;
    int             grp_len;
    char           *grp;

    grp_len = ficlStackPopInteger(vm->dataStack);
    grp = (char *) ficlStackPopPointer(vm->dataStack);
    grp[grp_len] = '\0';

    len = ficlStackPopInteger(vm->dataStack);
    tmp = (char *) ficlStackPopPointer(vm->dataStack);
    Mbox = ficlStackPopInteger(vm->dataStack);

    strncpy(mess, tmp, len);
    mess[len] = '\0';

    ret = SP_multicast(Mbox, AGREED_MESS, grp, 1, len, mess);

    if (ret < 0) {
        SP_error(ret);
    }
}

static void athSPPoll(ficlVm * vm) {
    mailbox         Mbox;
    int             ret;

    Mbox = ficlStackPopInteger(vm->dataStack);
    ret = SP_poll(Mbox);
    ficlStackPushInteger(vm->dataStack, ret);
}

static void athSPRecv(ficlVm * vm) {
    mailbox         Mbox;
    int             ret;

    int             service_type = 0;
    static char     mess[102400];
    char            sender[MAX_GROUP_NAME];
    char            target_groups[100][MAX_GROUP_NAME];
    int             num_groups;
    membership_info memb_info;

    int16           mess_type;
    int             endian_mismatch;
    int             i;
    int             exitFlag = 0;
    char           *buffer;
    int             buff_len;

    buff_len = ficlStackPopInteger(vm->dataStack);
    buffer = (char *) ficlStackPopPointer(vm->dataStack);
    Mbox = ficlStackPopInteger(vm->dataStack);

    do {
        ret = SP_receive(Mbox, &service_type, sender, 100, &num_groups, target_groups, &mess_type, &endian_mismatch, sizeof(mess), mess);

        if (ret < 0) {
            SP_error(ret);
        }

        if (Is_regular_mess(service_type)) {
            mess[ret] = 0;

            //
            // Add test, if message is longer than buffer, truncate.
            //
            strncpy(buffer, mess, ret);
            buffer[ret] = '\0';
            exitFlag = 1;
            ficlStackPushInteger(vm->dataStack, ret);
        } else if (Is_membership_mess(service_type)) {
            ret = SP_get_memb_info(mess, service_type, &memb_info);

            if (ret < 0) {
                SP_error(ret);
            } else {
                (void)strcpy(buffer,memb_info.changed_member);
                ficlStackPushInteger(vm->dataStack, strlen(memb_info.changed_member));

                exitFlag = 1;
            }
        }
        ficlStackPushInteger(vm->dataStack, mess_type);
    } while (!exitFlag);
}

#endif


#if (defined(LINUX) && defined(GPIO))
#warning "... GPIO ..."
#endif

static void athSleep(ficlVm *vm)
{
    (void) sleep(ficlStackPopInteger(vm->dataStack));
}

char prompt[32];
static void athSetPrompt(ficlVm *vm)
{
    //    extern char prompt[];
    char *ptr;
    int len;
    int crFlag = 0;

    memset(prompt,0x00, 32);
    //    bzero(prompt,32);

    crFlag = ficlStackPopInteger(vm->dataStack);
    len = ficlStackPopInteger(vm->dataStack);
    ptr = (char *)ficlStackPopPointer(vm->dataStack);

    strncpy(prompt,ptr,len);

    if( crFlag != 0) {
        strcat(prompt,"\n");
    }

}

static void athResetPrompt(ficlVm *vm)
{
    extern char prompt[];

    memset(prompt,0x00, 32);
    //    bzero(prompt,32);

    strcpy(prompt,FICL_PROMPT);
}


#ifdef MTHREAD
/*
   static void athCreateThread(ficlVm * vm)
   {
   ficlVm         *newVm;
   char           *ptr;
   int             len;
   int             i;
   pthread_t       t;
   int             status;


   newVm = ficlStackPopPointer(vm->dataStack);
   len = ficlStackPopInteger(vm->dataStack);
   ptr = ficlStackPopPointer(vm->dataStack);
   ptr[len] = '\0';

   ficlStackPushPointer(newVm->dataStack, ptr);
   ficlStackPushInteger(newVm->dataStack, len);



   status = pthread_create(&t,
   NULL,
   (void *) athExecuteThread,
   (void *) newVm);

//	ficlStackPushPointer(vm->dataStack, (void *) t);
ficlStackPushInteger(vm->dataStack, status);

}
*/

/*
   static void
   athYieldThread(ficlVm * vm)
   {
#ifdef _POSIX_PRIORITY_SCHEDULING
int             status;
#warning "Yield works"
#ifdef MAC
#warning "sched_yield"
status = sched_yield();
#else
#warning "pthread_yield"
status=pthread_yield();
#endif
//    usleep(1);
}
*/
    static void
athDeleteThread(ficlVm * vm)
{
    pthread_t       t;

    t = (pthread_t) ficlStackPopPointer(vm->dataStack);
}

    static void
athCreateMutex(ficlVm * vm)
{
    pthread_mutex_t *mutexp;

    mutexp = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mutexp, NULL);

    ficlStackPushPointer(vm->dataStack, mutexp);
}

    static void
athLockMutex(ficlVm * vm)
{
    pthread_mutex_t *mutexp;
    int             status;

    mutexp = ficlStackPopPointer(vm->dataStack);
    status = pthread_mutex_lock(mutexp);
    ficlStackPushInteger(vm->dataStack, status);
}

    static void
athUnlockMutex(ficlVm * vm)
{
    pthread_mutex_t *mutexp;
    int             status;

    mutexp = ficlStackPopPointer(vm->dataStack);
    status = pthread_mutex_unlock(mutexp);
    ficlStackPushInteger(vm->dataStack, status);
}


    static void
athTryLockMutex(ficlVm * vm)
{
    pthread_mutex_t *mutexp;
    int             status;

    mutexp = ficlStackPopPointer(vm->dataStack);
    status = pthread_mutex_trylock(mutexp);
    ficlStackPushInteger(vm->dataStack, status);
}
#endif

/* Stack -- socket */
#ifdef SOCKET

    static void
athSocket(ficlVm * vm)
{
    int             sock1;
    sock1 = socket(AF_INET, SOCK_STREAM, 0);
    ficlStackPushInteger(vm->dataStack, sock1);
}

//Stack:--socket
/*
   static void athLocalSocket(ficlVm *vm)
   {
   int sock1;

   sock1 =  socket(AF_UNIX, SOCK_STREAM, 0);
   ficlStackPushInteger(vm->dataStack,sock1);
   }
   */
/* Stack: addr len port -- flag */

    static void
athConnect(ficlVm * vm)
{
    char           *hostName;
    int             len, port;
    int             tmp;
    int             sock1;
    int             exitStatus = 0;
    struct sockaddr_in serv_addr;
    struct hostent *hp;

    port = ficlStackPopInteger(vm->dataStack);
    len = ficlStackPopInteger(vm->dataStack);
    hostName = (char *)ficlStackPopPointer(vm->dataStack);
    hostName[len] = '\0';

    hp = (struct hostent *) gethostbyname(hostName);
    memset((char *) &serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    memcpy((char *) &serv_addr.sin_addr, (void *) hp->h_addr, hp->h_length);

    sock1 = socket(AF_INET, SOCK_STREAM, 0);
    if (sock1 < 0) 	{
        exitStatus = -1;
    } else 	{
        tmp = connect(sock1, (struct sockaddr *) & serv_addr, sizeof(serv_addr));
        if (tmp < 0)
            exitStatus = -1;
    }

    if (exitStatus == 0) {
        ficlStackPushInteger(vm->dataStack, sock1);
    }
    ficlStackPushInteger(vm->dataStack, exitStatus);
}

/* Stack: port socket -- status */

    static void
athBind(ficlVm * vm)
{
    struct sockaddr_in serv_addr;
    int             port, status, sock1;
    struct hostent *hp;

    sock1 = ficlStackPopInteger(vm->dataStack);
    port = ficlStackPopInteger(vm->dataStack);

    memset((char *) &serv_addr, '\0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    status = bind(sock1, (struct sockaddr *) & serv_addr, sizeof(serv_addr));


    ficlStackPushInteger(vm->dataStack, status);
}

    static void
athListen(ficlVm * vm)
{
    int             sock1, status;

    sock1 = ficlStackPopInteger(vm->dataStack);
    status = listen(sock1, 5);
    ficlStackPushInteger(vm->dataStack, status);
}


    static void
athAccept(ficlVm * vm)
{
    int             sock1, status, clnt_len;
    int             sock2;

    struct sockaddr_in clnt_addr;

    sock1 = ficlStackPopInteger(vm->dataStack);
    sock2 = accept(sock1, (struct sockaddr *) & clnt_addr, (socklen_t *) & clnt_len);

    ficlStackPushInteger(vm->dataStack, sock2);
}

/* Stack: buffer len socket -- count */

static void athRecv(ficlVm * vm)
{
    int             n;
    int             sock2;
    int             len;
    int flag = 0;
    char           *msg;

    sock2 = ficlStackPopInteger(vm->dataStack);
    len = ficlStackPopInteger(vm->dataStack);
    msg =(char *)ficlStackPopPointer(vm->dataStack);
    n = recv(sock2, msg, len, 0);
    ficlStackPushInteger(vm->dataStack, n);
    //    flag = ( n < 0 );
    //	ficlStackPushInteger(vm->dataStack, flag);
}

    static void
athSend(ficlVm * vm)
{
    char           *buffer;
    int             len;
    int flag=0;
    int             sock2;
    int             status;

    sock2 = ficlStackPopInteger(vm->dataStack);
    len = ficlStackPopInteger(vm->dataStack);
    buffer = (char *)ficlStackPopPointer(vm->dataStack);

    status = send(sock2, buffer, len, 0);
    ficlStackPushInteger(vm->dataStack, status);
    flag = ( status < 0 );
    ficlStackPushInteger(vm->dataStack, flag);

}

    static void
athClose(ficlVm * vm)
{
    int             sock;

    sock = ficlStackPopInteger(vm->dataStack);
    close(sock);
}

#endif

/*
   arg O_NONBLOCK 0x004
   cmd F_SETFL 4
   */

static void athFdGet( ficlVm *vm) {
    int fd;
    ficlFile *ff;

    ff  = (ficlFile *)ficlStackPopPointer(vm->dataStack);
    fd = (int)ff->fd;

    ficlStackPushInteger(vm->dataStack, fd);
}

static void athFcntl(ficlVm *vm) {
    int cmd;
    int arg;
    int fd;
    int status = 0;

    //    ficlFile *ff;

    arg = ficlStackPopInteger(vm->dataStack);
    cmd = ficlStackPopInteger(vm->dataStack);
    //	ff  = ficlStackPopPointer(vm->dataStack);
    fd  = ficlStackPopInteger(vm->dataStack);

    //    fd = ff->fd;

    //    printf("Before Flags = %02x\n",fcntl(fd,F_GETFL,0) );
    status = fcntl(fd,cmd,arg);
    //    printf("After Flags = %02x\n",fcntl(fd,F_GETFL,0) );
    ficlStackPushInteger(vm->dataStack, status);

}

/*
   static void athIoctl(ficlVm *vm)
   {
   int fd;
   unsigned long cmd;
   void *arg;

   arg = (void *)ficlStackPopInteger(vm->dataStack);
   cmd = (unsigned long)ficlStackPopInteger(vm->dataStack);
   fd  = (int)ficlStackPopInteger(vm->dataStack);
   }
   */

    static void
athSeal(ficlVm * vm)
{
    vm->sealed = -1;
}

    static void
athUnseal(ficlVm * vm)
{
    vm->sealed = 0;
}

/*
 * Investigate using sysctl as it may be more portable
 * than uname.
 */

// #ifndef ARM
#ifndef EMBEDDED
static void athUname(ficlVm * vm) {
    struct utsname  buf;
    int             res;
    //    extern char    *loadPath;

    res = uname(&buf);
    printf("System name :%s\n", buf.sysname);
    printf("Host name   :%s\n", buf.nodename);
    printf("Release     :%s\n", buf.release);
    printf("Version     :%s\n", buf.version);
    printf("Machine     :%s\n", buf.machine);
    //    printf("Load        :%s\n", loadPath);
}
#endif

int verbose;

static void athVerboseQ(ficlVm *vm) {
    ficlStackPushInteger(vm->dataStack, verbose);
}
/*
#define OS_UNKNOWN 0
#define OS_LINUX 1
#define OS_DARWIN 2
#define OS_FREEBSD 3
#define OS_SOLARIS 4
#define OS_UCLINUX 5
#define OS_QNX 6
*/

/*
#define CPU_UNKNOWN 0
#define CPU_X86 1
#define CPU_PPC 2
#define CPU_8XX 3
#define CPU_AMD64 4
#define CPU_SPARC 5
#define CPU_COLDFIRE 6
#define CPU_ARM 7
#define CPU_MIPS 8
#define CPU_SH4A 9
*/

// #ifndef ARM
static void athCpu(ficlVm * vm) {
    int             cpu = CPU_UNKNOWN;
#ifndef EMBEDDED
    struct utsname  buf;
    int             res;

    res = uname(&buf);

    //    printf("%s\n",buf.machine);

    if ((strcasecmp(buf.machine, "i686") == 0) || (strcasecmp(buf.machine, "i386") == 0) || (strcasecmp(buf.machine, "x86pc") == 0) || (strcasecmp(buf.machine,"x86_64") == 0) )
    {
        cpu = CPU_X86;
    } else if ( (strcasecmp(buf.machine, "Power Macintosh") == 0) || (strcasecmp(buf.machine, "ppc") == 0) )
    {
        cpu = CPU_PPC;
    } else if ((strcasecmp(buf.machine, "armv5tel") == 0) || !strcmp(buf.machine,"armv7l") || !strcmp(buf.machine,"armv6l")) {
        cpu = CPU_ARM;
    } else if (strcasecmp(buf.machine, "m68knommu") == 0) {
        cpu = CPU_COLDFIRE;
    } else if (strcasecmp(buf.machine, "mips") == 0) {
        cpu = CPU_MIPS;
    } else if (strcasecmp(buf.machine, "sh4a") == 0) {
        cpu = CPU_SH4A;
    }
#else
    cpu = CPU_ARM;
#endif

    ficlStackPushInteger(vm->dataStack, cpu);

}

static void athOs(ficlVm * vm) {
    int             os = OS_UNKNOWN;
#ifndef EMBEDDED
    struct utsname  buf;
    int             res;

    res = uname(&buf);

    //    printf("%s\n",buf.sysname);

    if (strcasecmp(buf.sysname, "linux") == 0) {
        os = OS_LINUX;
    } else if (strcasecmp(buf.sysname, "darwin") == 0) {
        os = OS_DARWIN;
    } else if (strcasecmp(buf.sysname, "uClinux") == 0) {
        os = OS_UCLINUX;
    }  else if (strcasecmp(buf.sysname, "QNX") == 0) {
        os = OS_QNX;
    }

#else
    os = OS_UNKNOWN;
#endif
    ficlStackPushInteger(vm->dataStack, os);
}

static void athLinuxQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_LINUX );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athDarwinQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_DARWIN );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athBSDQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_FREEBSD );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athSolarisQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_SOLARIS );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}
static void athUcLinuxQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_UCLINUX );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athQNXQ(ficlVm *vm) {
    int n;

    athOs(vm);
    n=( ficlStackPopInteger(vm->dataStack) == OS_QNX );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athX86Q(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_X86 );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athPPCQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_PPC );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void ath8XXQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_8XX );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athAMD64Q(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_AMD64 );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athSparcQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_SPARC  );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athColdQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_COLDFIRE  );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athARMQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_ARM  );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athMIPSQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_MIPS  );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}

static void athSH4AQ(ficlVm *vm) {
    int n;

    athCpu(vm);
    n=( ficlStackPopInteger(vm->dataStack) == CPU_SH4A  );
    if( n !=0 ) {
        n=-1;
    }
    ficlStackPushInteger(vm->dataStack, n);
}



/*
   Return the hostname at pad, and the length on the stack.
   */
// #ifndef ARM
static void athHostname(ficlVm * vm) {
    int             n;
#ifndef EMBEDDED
    struct utsname  buf;
    int             res;
    int             len;
    char           *dest;
    char *tmp;

    /*
       len = ficlStackPopInteger(vm->dataStack);
       dest = ficlStackPopPointer(vm->dataStack);
       */

    res = uname(&buf);

    tmp = (char *)strtok(buf.nodename,".");

    /*
       if (len >= strlen(buf.nodename))
       {
       n = strlen(buf.nodename);
       } else
       {
       n = len;
       }
       */

    n = strlen(buf.nodename);
    //	strncpy((char *) dest, (char *) buf.nodename, (size_t) n);
    strncpy((char *) vm->pad, (char *) buf.nodename, (size_t) n);
#else
    n=0;
#endif
    ficlStackPushInteger(vm->dataStack, n);
}
// #endif
#ifdef LIBMODBUS
// #define MODBUS_DEBUG

static void athModBusNew(ficlVm * vm) {
    modbus_t *ctx;
    char *tty;

    int tty_len=0;
    int baud_rate=9600;
    char parity='N';
    int length=8;
    int stop_bits=2;

    tty_len = ficlStackPopInteger( vm->dataStack );

    tty= ficlStackPopPointer( vm->dataStack );
    tty[tty_len]=0;

    printf("\nFIXME: Can't change serial port settings !\n\n");
    printf("Serial Port:%s\n", tty);
    printf("Baud Rate  :%d\n",baud_rate);
    printf("Parity     :%c\n",parity);
    printf("Word length:%d\n", length);
    printf("Stop bits  :%d\n\n", stop_bits);    

    ctx = modbus_new_rtu(tty, baud_rate, parity, length, stop_bits);

    if ( ctx != NULL ) {
        ficlStackPushPointer(vm->dataStack, ctx);
        ficlStackPushInteger(vm->dataStack,0);
    } else {
        ficlStackPushInteger(vm->dataStack,-1);
    }

}
//
// Stack: addr len port
// Desc: Pass a string, holding the IP address, and the port.
// e.g. 192.168.0.190 502 
//
static void athModBusNewNet(ficlVm *vm) {
    modbus_t *ctx;
    int port;
    int len;
    char *address;
    int status=-1;

    port = ficlStackPopInteger( vm->dataStack);
    len = ficlStackPopInteger( vm->dataStack);

    address = ficlStackPopPointer( vm->dataStack );

    address[len]=(char ) NULL;

    ctx = modbus_new_tcp(address, port);

#ifdef MODBUS_DEBUG
    printf("modbus_new_tcp\n");
    printf("\tAddress:%s\n",address);
    printf("\tPort   :%d\n",port);
#endif

    if (ctx == 0) {
        ficlStackPushInteger(vm->dataStack,-1);
    } else {
        ficlStackPushPointer(vm->dataStack,ctx);
        ficlStackPushInteger(vm->dataStack,0);
    }

}
static void athModBusConnect(ficlVm * vm) {
    modbus_t *ctx;

    ctx= ficlStackPopPointer( vm->dataStack );

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n",modbus_strerror(errno));
        modbus_free(ctx);
    }
}
/*
   Set ModBus RTU id.  Beahviour is different if the target is a RTU or TCP node, from the
   libmodbus docs:

 *RTU*::
 Define the slave ID of the remote device to talk in master mode or set the
 internal slave ID in slave mode. According to the protocol, a Modbus device must
 only accept message holing its slave number or the special broadcast number.

 *TCP*::
 The slave number is only required in TCP if the message must reach a device
 on a serial network. The special value 'MODBUS_TCP_SLAVE' (0xFF) can be used in TCP mode to restore
 the default value.

*/
static void athModBusSetSlave(ficlVm * vm) {
    modbus_t *ctx;
    int rtu;

    rtu=ficlStackPopInteger(vm->dataStack);
    ctx=ficlStackPopPointer(vm->dataStack);

    modbus_set_slave(ctx, rtu);
}

/*
 * Return current timeout im milliseconds.
 */
static void athModBusGetTimeout(ficlVm * vm) {
    modbus_t *ctx;
    struct timeval response_timeout;
    int time;

    ctx=ficlStackPopPointer(vm->dataStack);
    modbus_get_response_timeout(ctx, &response_timeout);

    time = (response_timeout.tv_sec * 1000) + (response_timeout.tv_usec/1000) ;
    ficlStackPushInteger(vm->dataStack, time );
}

/*
 * Set time out.
 * IN: ctx ms
 * OUT: --
 */
static void athModBusSetTimeout(ficlVm * vm) {
    modbus_t *ctx;
    struct timeval response_timeout;
    int time;
    int timeS;
    int timeUs;

    time=ficlStackPopInteger(vm->dataStack);
    ctx=ficlStackPopPointer(vm->dataStack);

    timeS = time / 1000;
    response_timeout.tv_sec = timeS;

    timeUs = ((time - (timeS * 1000)) * 1000);

    response_timeout.tv_usec = timeUs;
    modbus_set_response_timeout(ctx, &response_timeout);
}

/*
 * Modbus function code 0x02 (read input status).
 */

static void athModBusReadInputBits(ficlVm * vm) {
    modbus_t *ctx;

    int addr=0;
    int nb_points=1;
    uint8_t *tab_rp_registers;
    int rc=0;
    int status=0;

    tab_rp_registers = ( uint8_t *)ficlStackPopPointer(vm->dataStack);
    nb_points = ficlStackPopInteger( vm->dataStack);
    addr = ficlStackPopInteger( vm->dataStack);
    ctx = ( modbus_t *) ficlStackPopPointer(vm->dataStack);    

    rc=modbus_read_input_bits(ctx, addr, nb_points, tab_rp_registers);

    if (rc < 0) {
        status = -1;
    } else {
        status = 0;
        ficlStackPushInteger(vm->dataStack,rc);
    }
    ficlStackPushInteger(vm->dataStack,status);

}
/*
 * Modbus function code 0x03 (read holding registers).
 */
static void athModBusReadRegisters(ficlVm * vm) {
    modbus_t *ctx;

    int addr=0;
    int nb_points=1;
    uint16_t *tab_rp_registers;
    int rc=0;
    int status=0;

    tab_rp_registers = ( uint16_t *)ficlStackPopPointer(vm->dataStack);
    nb_points = ficlStackPopInteger( vm->dataStack);
    addr = ficlStackPopInteger( vm->dataStack);
    ctx = ( modbus_t *) ficlStackPopPointer(vm->dataStack);

    rc=modbus_read_registers(ctx, addr, nb_points, tab_rp_registers);

    if (rc < 0) {
        status = -1;
    } else {
        status = 0;
        ficlStackPushInteger(vm->dataStack,rc);
    }
    ficlStackPushInteger(vm->dataStack,status);
}
/*
 * Modbus function code 0x04 (read input registers).
 */
static void athModBusReadInputRegisters(ficlVm * vm) {
    modbus_t *ctx;

    int addr=0;
    int nb_points=1;
    uint16_t *tab_rp_registers;
    int rc=0;
    int status=0;

    tab_rp_registers = ( uint16_t *)ficlStackPopPointer(vm->dataStack);
    nb_points = ficlStackPopInteger( vm->dataStack);
    addr = ficlStackPopInteger( vm->dataStack);
    ctx = ( modbus_t *) ficlStackPopPointer(vm->dataStack);

    rc=modbus_read_input_registers(ctx, addr, nb_points, tab_rp_registers);

    if (rc < 0) {
        status = -1;
    } else {
        status = 0;
        ficlStackPushInteger(vm->dataStack,rc);
    }
    ficlStackPushInteger(vm->dataStack,status);
}
/*
 * Modbus function code 0x05 (Write single coil)
 */
static void athModbusWriteSingleBit( ficlVm *vm) {

    modbus_t *ctx;
    int bitAddress;
    int state;
    int rc=-1;

    state = ficlStackPopInteger( vm->dataStack);
    bitAddress = ficlStackPopInteger( vm->dataStack);
    ctx = ( modbus_t *) ficlStackPopPointer(vm->dataStack);

    if (state == 0) {
        state = OFF;
    } else {
        state = ON;
    }
    rc = modbus_write_bit( ctx, bitAddress, state);
    modbus_flush( ctx );

    if (1 == rc) {
        ficlStackPushInteger(vm->dataStack,0);
    } else {
        ficlStackPushInteger(vm->dataStack,-1);
    }
}
/*
 * Modbus function code 0x06 (write single register)
 */
static void athModBusWriteRegister( ficlVm *vm) {
    modbus_t *ctx;

    int addr=0;
    int data=-1;
    int status=-1;

    data = ficlStackPopInteger( vm->dataStack);
    addr = ficlStackPopInteger( vm->dataStack);

    ctx = ( modbus_t *) ficlStackPopPointer(vm->dataStack);

    status = modbus_write_register(ctx,addr,data);

    if (1 == status) {
        ficlStackPushInteger(vm->dataStack,0);
    } else {
        ficlStackPushInteger(vm->dataStack,-1);
    }


}

/*
 *
 * IN: context address ptr reg-count
 * OUT:
 */
static void athModBusWriteMultipleRegisters( ficlVm *vm) {
    modbus_t *ctx;

    uint16_t *ptr;
    int len;
    int addr=0;

    int status=-1;

    len = ficlStackPopInteger( vm->dataStack);
    ptr = ficlStackPopPointer( vm->dataStack);
    addr = ficlStackPopInteger( vm->dataStack);

    ctx = ( modbus_t *) ficlStackPopPointer(vm->dataStack);

    status = modbus_write_registers(ctx,addr,len,ptr);

    if (-1 == status) {
        ficlStackPushInteger(vm->dataStack,-1);
    } else {
        ficlStackPushInteger(vm->dataStack,status);
        ficlStackPushInteger(vm->dataStack,0);
    }


}

static void athModBusClose(ficlVm * vm) {
    modbus_t *ctx;

    ctx = ( modbus_t *) ficlStackPopPointer(vm->dataStack);

    modbus_close( ctx );
    modbus_free ( ctx );

}

static void athModBusDebug( ficlVm *vm) {
    modbus_t *ctx;

    int flag;
    int status=0;

    flag =  ficlStackPopInteger( vm->dataStack);
    ctx = ( modbus_t *) ficlStackPopPointer(vm->dataStack);

    if (flag == 0) {
        modbus_set_debug(ctx, FALSE);
    } else {
        modbus_set_debug(ctx, TRUE);
    }
}

static void athModBusErrorRecovery( ficlVm *vm) {
    modbus_t *ctx;

    int flag;
    int status=0;

    flag =  ficlStackPopInteger( vm->dataStack);
    ctx = ( modbus_t *) ficlStackPopPointer(vm->dataStack);

    if (flag == 0) {
        modbus_set_error_recovery(ctx,0);
    } else {
        modbus_set_error_recovery(ctx,
                MODBUS_ERROR_RECOVERY_LINK |
                MODBUS_ERROR_RECOVERY_PROTOCOL);
    }

}

static void athModBusPrintError( ficlVm *vm) {
    int modbus_errno;

    //    modbus_errno = ficlStackPopInteger( vm->dataStack);

    fprintf(stderr,"%s\n",modbus_strerror( errno ));

}

#endif

#ifdef MODBUS
static unsigned char auchCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
    0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80,
    0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
    0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
    0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80,
    0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
    0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

/* Table of CRC values for low–order byte */
static char     auchCRCLo[] = {0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA,
    0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17,
    0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33,
    0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9,
    0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24,
    0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0,
    0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8,
    0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD,
    0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1,
    0x71, 0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B,
    0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E,
    0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82,
    0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

    static void
athCalcCRC(ficlVm * vm)
{
    /* The function returns the CRC as a unsigned short type  */
    /* *puchMsg message to calculate CRC upon  */
    /* usDataLen  quantity of bytes in message   */

    unsigned char  *puchMsg;
    unsigned short  usDataLen;
    unsigned char   uchCRCHi = 0xFF;	/* high byte of CRC
                                         * initialized   */
    unsigned char   uchCRCLo = 0xFF;	/* low byte of CRC
                                         * initialized   */
    unsigned        uIndex;	/* will index into CRC lookup table   */

    usDataLen = (unsigned short) ficlStackPopInteger(vm->dataStack);
    puchMsg = (unsigned char *) ficlStackPopPointer(vm->dataStack);

    while (usDataLen--)	/* pass through message buffer   */
    {
        uIndex = uchCRCLo ^ *puchMsg++;	/* calculate the CRC   */
        uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
        uchCRCHi = auchCRCLo[uIndex];
    }
    /*
     * ficlStackPushInteger (vm->dataStack,uchCRCHi << 8 | uchCRCLo) ;
     */
    ficlStackPushInteger(vm->dataStack, uchCRCHi);
    ficlStackPushInteger(vm->dataStack, uchCRCLo);
}

#endif

#ifdef I2C
#warning "... I2C"
void athI2cOpen(ficlVm *vm) {
    char *portName;
    int len;
    int fd;

    len = ficlStackPopInteger(vm->dataStack);
    portName = ficlStackPopPointer(vm->dataStack);

    portName[len]='\0';

    printf("Portname = %s\n",portName);

    fd = i2c_open_port( portName);
    ficlStackPushInteger(vm->dataStack, fd);
}

void athI2cWrite(ficlVm *vm) {
    int len;
    char *data;
    int reg;
    char addr;
    int fd;
    int ret;

    len=ficlStackPopInteger(vm->dataStack);
    data = ficlStackPopPointer(vm->dataStack);
    reg = ficlStackPopInteger(vm->dataStack);
    addr = (char) (ficlStackPopInteger(vm->dataStack) & 0xff);
    fd = ficlStackPopInteger(vm->dataStack);;
    ret=i2c_write( fd, addr, reg, data, len);
    ficlStackPushInteger(vm->dataStack,ret);
}

void athI2cRead(ficlVm *vm) {
    int len;
    char *data;
    int reg;
    char addr;
    int fd;
    int ret;

    len=ficlStackPopInteger(vm->dataStack);
    data = ficlStackPopPointer(vm->dataStack);
    reg = ficlStackPopInteger(vm->dataStack);
    addr = (char) (ficlStackPopInteger(vm->dataStack) & 0xff);
    fd = ficlStackPopInteger(vm->dataStack);;
    ret=i2c_read( fd, addr, reg, data, len);
    ficlStackPushInteger(vm->dataStack,ret);
}

void athI2cClose(ficlVm *vm) {
    int fd;

    i2c_close( ficlStackPopInteger(vm->dataStack) );
}
#endif

#ifdef LINUX
#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)
/*
Stack: <phys address> --- <false>| addr <true>
*/
void athMmap(ficlVm *vm) {
    int fd;
    int size;
    off_t target;
    int ret=0;
    void *map_base;

    fd = open("/dev/mem", O_RDWR | O_SYNC);

    if( fd == -1) {
        ret=-1;
    } else {
        size = ficlStackPopInteger(vm->dataStack) ;
        target = ficlStackPopInteger(vm->dataStack) ;

        //        map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
        map_base = mmap(0, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);

        if(map_base == (void *) -1) {
            ret=-1;
        } else {
            ret=0;
            ficlStackPushPointer(vm->dataStack, map_base);
        }

        ficlStackPushInteger(vm->dataStack,ret);
    }

}

#endif

#ifdef LINUX
void athIoctl(ficlVm *vm) {

    int fd=-1;
    void *cmd = (void *)-1;
    int data = -1;
    int ret = -1;

    data = ficlStackPopInteger(vm->dataStack) ;
    cmd = (void *)ficlStackPopInteger(vm->dataStack) ;
    fd = ficlStackPopInteger(vm->dataStack) ;

    ret = ioctl(fd, cmd, data);
    ficlStackPushInteger(vm->dataStack,ret);
}
#endif

#ifdef SERIAL
#warning "Serial comms selected."
/*
 * Stack: PortName len speed.
 */
void athOpenSerialPort(ficlVm *vm) {
    int speed;
    char *portName;
    int len;
    int res=0;
    ficlFile *serPort;

    speed = ficlStackPopInteger(vm->dataStack) ;
    len = ficlStackPopInteger(vm->dataStack) ;
    portName = ficlStackPopPointer(vm->dataStack);

    res = u16OpenSerialPort(portName,speed);

    serPort = (ficlFile *)malloc(sizeof(struct ficlFile));

    serPort->fd = res ;
    serPort->f = fdopen(res,"r+");
    strcpy( serPort->filename, portName );

    ficlStackPushInteger(vm->dataStack,serPort);
}
#endif

#ifdef LINUX
static void athPrimitiveTick(ficlVm *vm) {
    /*
       ficlCell *pTick;

       FICL_STACK_CHECK(vm->dataStack, 0, 1);

       pTick = (ficlCell *)(&vm->tickCounter);
       ficlStackPush(vm->dataStack, FICL_LVALUE_TO_CELL(pTick));
       return;
       */

    ficlStackPushInteger(vm->dataStack, systemTick);

}

void timer_handler(int signum) {
    printf("... tick ...\n");
    systemTick++;
}

static void athStartTimer(ficlVm *vm) {
    struct sigaction sa;
    struct itimerval timer;

    int interval=10;

    /* get interval in ms */
    interval = ficlStackPopInteger( vm->dataStack );

    memset(&sa,0,sizeof(sa));
    sa.sa_handler = &timer_handler;
    sigaction(SIGVTALRM, &sa, NULL);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = interval * 1000;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = interval * 1000;

    setitimer(ITIMER_VIRTUAL, &timer, NULL);

}
#endif

#ifdef REDIS
void athConnectRedis(ficlVm *vm) {
    redisContext *c;
    int delay;
    int status=0;

    char *ipAddress;
    int ipLen;
    int port;
    struct timeval timeout;

    int timer; // In seconds

    timer = ficlStackPopInteger( vm->dataStack );
    port = ficlStackPopInteger( vm->dataStack );
    ipLen = ficlStackPopInteger( vm->dataStack );
    ipAddress = (char *)ficlStackPopPointer( vm->dataStack );

    timeout.tv_sec = timer;
    timeout.tv_usec=0;

    c = redisConnectWithTimeout((char*)ipAddress, port, timeout);

    status = c->err;

    if( status == 0) {
        ficlStackPushPointer(vm->dataStack, c);
        ficlStackPushInteger(vm->dataStack, 0 );
    } else {
        ficlStackPushPointer(vm->dataStack,c->errstr);
        ficlStackPushInteger(vm->dataStack, strlen(c->errstr)  );
        ficlStackPushInteger(vm->dataStack, c->err );
        ficlStackPushInteger(vm->dataStack, -1 );
    }
}

void athRedisPing(ficlVm *vm) {
    redisContext *c;
    redisReply *reply;

    c = (redisContext *)ficlStackPopPointer(vm->dataStack);
    reply = (redisReply *)redisCommand(c,"PING");

    ficlStackPushInteger(vm->dataStack, strcmp(reply->str,"PONG"));
    freeReplyObject(reply);
}

void athRedisFreeReply(ficlVm *vm) {
    redisReply *reply;

    reply = (redisReply *)ficlStackPopPointer(vm->dataStack) ;

    freeReplyObject(reply);
}

void athRedisGetElements(ficlVm *vm) {
    redisReply *reply;

    reply = (redisReply *)ficlStackPopPointer(vm->dataStack) ;

    ficlStackPushInteger(vm->dataStack, reply->elements);
}

void athRedisGetElement(ficlVm *vm) {

    int idx;
    redisReply *reply;
    char *ptr;

    idx = ficlStackPopInteger( vm->dataStack );

    reply = (redisReply *)ficlStackPopPointer(vm->dataStack) ;

    ptr = reply->element[idx]->str;

    ficlStackPushPointer(vm->dataStack, ptr );
    if ( ptr != (char *)NULL) {
        ficlStackPushInteger(vm->dataStack, strlen( ptr ) );
    } else {
        ficlStackPushInteger(vm->dataStack, 0 );
    }
}

void athRedisContextErr(ficlVm *vm) {
    redisContext *c;

    c = (redisContext *)ficlStackPopPointer(vm->dataStack) ;
    //    ficlStackPushPointer(vm->dataStack, c->err );
    ficlStackPushInteger(vm->dataStack, c->err );
}

void athRedisContextErrMsg(ficlVm *vm) {
    redisContext *c;

    c = (redisContext *)ficlStackPopPointer(vm->dataStack) ;
    ficlStackPushPointer(vm->dataStack, c->errstr );
    ficlStackPushInteger(vm->dataStack, strlen(c->errstr) );
}

void athRedisContextFlags(ficlVm *vm) {
    redisContext *c;

    c = (redisContext *)ficlStackPopPointer(vm->dataStack) ;
    //    ficlStackPushPointer(vm->dataStack, c->flags );
    ficlStackPushInteger(vm->dataStack, c->flags );
}

void athRedisCmd(ficlVm *vm) {
    redisContext *c;
    redisReply *reply;

    int len;
    char *cmd;

    len = ficlStackPopInteger(vm->dataStack) ;
    cmd = (char *)ficlStackPopPointer(vm->dataStack) ;
    c   = (redisContext *)ficlStackPopPointer(vm->dataStack) ;

    reply = (redisReply *)redisCommand(c,cmd);
    ficlStackPushPointer(vm->dataStack, reply );

    ficlStackPushInteger(vm->dataStack, ( reply == 0  ) );
}

void athRedisGetString(ficlVm *vm) {
    redisReply *reply;

    reply = (redisReply *)ficlStackPopPointer(vm->dataStack) ;
    ficlStackPushPointer(vm->dataStack, reply->str );
    ficlStackPushInteger(vm->dataStack, reply->len );
}

void athRedisGetType(ficlVm *vm) {
    redisReply *reply;

    reply = (redisReply *)ficlStackPopPointer(vm->dataStack) ;
    ficlStackPushInteger(vm->dataStack, reply->type );
}

void athRedisGetInteger(ficlVm *vm) {
    redisReply *reply;

    reply = (redisReply *)ficlStackPopPointer(vm->dataStack) ;
    ficlStackPushInteger(vm->dataStack, reply->type );
}

void athRedisDisplayReply(ficlVm *vm) {
    redisReply *reply;

    reply = (redisReply *)ficlStackPopPointer(vm->dataStack) ;

    printf("Type      : %d\n", reply->type);
    switch(reply->type) {
        case REDIS_REPLY_STRING:
            printf("\tString\n");
            printf("Length    : %d\n", reply->len);
            printf("\t>%s<\n",reply->str);
            break;
        case REDIS_REPLY_ARRAY:
            printf("\tArray\n");
            break;
        case REDIS_REPLY_INTEGER:
            printf("\tInteger\n");
            break;
        case REDIS_REPLY_NIL:
            printf("\tNill\n");
            break;
        case REDIS_REPLY_STATUS:
            printf("\tStatus\n");
            break;
        case REDIS_REPLY_ERROR:
            printf("\tError\n");
            break;
    }
}
#endif

#if FICL_WANT_FILE
#if LINUX
#include <sys/inotify.h>
#endif

void athRead(ficlVm *vm) {
    ssize_t size;
    size_t count;
    void *buf;
    int fd;
    extern int errno;

    count = ficlStackPopInteger(vm->dataStack);
    buf   = (void *)ficlStackPopPointer(vm->dataStack) ;
    fd    = ficlStackPopInteger(vm->dataStack);

    size = read(fd,buf,count);
    if( size < 0) {
        ficlStackPushInteger(vm->dataStack, errno);
        ficlStackPushInteger(vm->dataStack, -1);
    } else {
        ficlStackPushInteger(vm->dataStack, size);
        ficlStackPushInteger(vm->dataStack, 0);
    }

}

void athWrite(ficlVm *vm) {
    ssize_t size;
    size_t count;
    void *buf;
    int fd;
    extern int errno;

    count = ficlStackPopInteger(vm->dataStack);
    buf   = (void *)ficlStackPopPointer(vm->dataStack) ;
    fd    = ficlStackPopInteger(vm->dataStack);

    size = write(fd,buf,count);
    if( (size < 0) || ( (size == 0) && (errno !=0 ) ) ) {
        ficlStackPushInteger(vm->dataStack, errno);
        ficlStackPushInteger(vm->dataStack, -1);
    } else {
        ficlStackPushInteger(vm->dataStack, size);
        ficlStackPushInteger(vm->dataStack, 0);
    }
}

#ifdef LINUX
/**
 * inotify_init() call
 *
 * @param None
 * @return on success - fd 0
 * @return on failure - -1
 *
 */
void athInotifyInit(ficlVm *vm) {
    int fd;

    fd = inotify_init();
    if(fd) {
        ficlStackPushInteger(vm->dataStack, fd);
        ficlStackPushInteger(vm->dataStack, 0);
    } else {
        ficlStackPushInteger(vm->dataStack, -1);
    }
}

/*
 * inotify_add_watch() call
 *
 * @param[in] fd 
 * @param[in[ pathname len 
 * @param[in] mask
 * @return on success - wd 0
 * @return on failure - -1
 *
 */
void athInotifyAddWatch(ficlVm *vm) {
    int fd;
    char *path;
    int len;
    int mask;
    int wd;

    mask = ficlStackPopInteger(vm->dataStack);
    len = ficlStackPopInteger(vm->dataStack);
    path = (void *)ficlStackPopPointer(vm->dataStack) ;
    fd = ficlStackPopInteger(vm->dataStack);

    path[len]='\0';

    wd = inotify_add_watch(fd,path,mask);

    if(wd < 0 ) {
        ficlStackPushInteger(vm->dataStack, -1);
    } else {
        ficlStackPushInteger(vm->dataStack, wd);
        ficlStackPushInteger(vm->dataStack, 0);
    }
}
/*
 * inotify_rm_watch() call
 *
 * @param[in] fd 
 * @param[in] wd
 * @return on success -  0
 * @return on failure - -1
 *
 * inotify_rm_watch( fd, wd );
 */
void athInotifyRmWatch(ficlVm *vm) {
    int wd;
    int fd;
    int res;

    wd = ficlStackPopInteger(vm->dataStack);
    fd = ficlStackPopInteger(vm->dataStack);

    res = inotify_rm_watch( fd, wd );

    ficlStackPushInteger(vm->dataStack, res);
}
#endif
#endif

void ficlSystemCompileExtras(ficlSystem * system) {
    ficlDictionary *dictionary = ficlSystemGetDictionary(system);

    //    ficlDictionarySetPrimitive(dictionary, (char *)"break", ficlPrimitiveBreak, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"get-pid", athGetPid, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"verbose?", athVerboseQ, FICL_WORD_DEFAULT);
#ifdef FICL_WANT_FILE
#ifdef LINUX
    ficlDictionarySetPrimitive(dictionary, (char *)"inotify-init",  athInotifyInit,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"inotify-add-watch",  athInotifyAddWatch,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"inotify-rm-watch",  athInotifyRmWatch,  FICL_WORD_DEFAULT);
#endif
    ficlDictionarySetPrimitive(dictionary, (char *)"read",  athRead,  FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"write", athWrite, FICL_WORD_DEFAULT);
#endif

    //    ficlDictionarySetPrimitive(dictionary, (char *)"key", athGetkey, FICL_WORD_DEFAULT);
    //    ficlDictionarySetPrimitive(dictionary, (char *)"?key", athQkey, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"stdout-flush", athStdoutFlush, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"zmove", athZmove, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"add-cr", athAddCr, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"set-prompt", athSetPrompt, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"reset-prompt", athResetPrompt, FICL_WORD_DEFAULT);

#ifndef EMBEDDED
    ficlDictionarySetPrimitive(dictionary, "popen", athPopenRWE, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "pclose", athPcloseRWE, FICL_WORD_DEFAULT);
#endif
    /*
#ifdef GDBM
ficlDictionarySetPrimitive(dictionary, "gdbm-open", athGdbmOpen, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "gdbm-close", athGdbmClose, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "gdbm-insert", athGdbmInsert, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "gdbm-update", athGdbmUpdate, FICL_WORD_DEFAULT);

ficlDictionarySetPrimitive(dictionary, "gdbm-fetch", athGdbmFetch, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "gdbm-delete", athGdbmDelete, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "gdbm-dump", athGdbmDump, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "gdbm-reorg", athGdbmReorganize, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "gdbm-sync", athGdbmSync, FICL_WORD_DEFAULT);
#endif
*/

    /*
#ifdef SIMPL
ficlDictionarySetPrimitive(dictionary, "simpl-attach", athSimplAttach, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "simpl-locate", athSimplLocate, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "simpl-send", athSimplSend, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "simpl-receive", athSimplReceive, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "simpl-reply", athSimplReply, FICL_WORD_DEFAULT);

ficlDictionarySetPrimitive(dictionary, "simpl-detach", athSimplDetach, FICL_WORD_DEFAULT);
#endif
*/

    //    ficlDictionarySetPrimitive(dictionary, (char *)"load", ficlPrimitiveLoad, FICL_WORD_DEFAULT);
    //    ficlDictionarySetPrimitive(dictionary, (char *)"$load", ficlDollarPrimitiveLoad, FICL_WORD_DEFAULT);
    //    ficlDictionarySetPrimitive(dictionary, (char *)"$load-dir", ficlDollarPrimitiveLoadDir, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"spewhash", ficlPrimitiveSpewHash, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"system", ficlPrimitiveSystem, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"$system", athPrimitiveDollarSystem, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"elapsed", athElapsed, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"now", athNow, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"time", athTime, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"date", athDate, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)".features", athFeatures, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"(int)", athSizeofInt, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"(char)", athSizeofChar, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"(char*)", athSizeofCharPtr, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"getenv", athGetenv, FICL_WORD_DEFAULT);
#ifndef EMBEDDED
    ficlDictionarySetPrimitive(dictionary, (char *)"signal", athSignal, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"kill", athKill, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"alarm", athSetAlarm, FICL_WORD_DEFAULT);
#endif
    ficlDictionarySetPrimitive(dictionary, (char *)"last-signal", getLastSignal, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"perror", athPerror, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"clr-errno", athClrErrno, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"errno", athGetErrno, FICL_WORD_DEFAULT);
#ifdef SYSV_IPC    
    ficlDictionarySetPrimitive(dictionary, (char *)"relsem", athRelSem, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"getsem", athGetSem, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"setsemvalue", athSetSemValue, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"getsemvalue", athGetSemValue, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"rmsem", athRmSem, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"semtran", athSemTran, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"shmat", athShmat, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"shmdt", athShmdt, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"shmrm", athShmrm, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"shmget", athShmGet, FICL_WORD_DEFAULT);
#endif    
    ficlDictionarySetPrimitive(dictionary, (char *)"ms", athMs, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"dump-fd",athFiclFileDump, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"sleep", athSleep, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"strtok", athStrTok, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"strsave", athStrsave, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"/string", athSlashString, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"-trailing", athMinusTrailing, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"2rot", athTwoRot, FICL_WORD_DEFAULT);
    //    #ifndef ARM
#ifndef EMBEDDED
    ficlDictionarySetPrimitive(dictionary, (char *)"uname", athUname, FICL_WORD_DEFAULT);
#endif
    ficlDictionarySetPrimitive(dictionary, (char *)"os", athOs, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"linux?", athLinuxQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"bsd?", athBSDQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"darwin?", athDarwinQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"solaris?", athSolarisQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"uclinux?", athUcLinuxQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"QNX?", athQNXQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"x86?", athX86Q, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"ppc?", athPPCQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"8xx?", ath8XXQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"amd64?", athAMD64Q, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"sparc?", athSparcQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"coldfire?", athColdQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"arm?", athARMQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"mips?", athMIPSQ, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"sh4a?", athSH4AQ, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"cpu", athCpu, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"hostname", athHostname, FICL_WORD_DEFAULT);
    //    #endif
#ifdef DYNLIB
    ficlDictionarySetPrimitive(dictionary, "dlopen", athDlOpen, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "dlclose", athDlClose, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "dlsym", athDlSym, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "dlerror", athDlError, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "dlexec", athDlExec, FICL_WORD_DEFAULT);
#endif

#ifdef DB
    ficlDictionarySetPrimitive(dictionary, "sqlite-open", athSqliteOpen, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "sqlite-compile", athSqliteCompile, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "sqlite-fetch", athSqliteFetch, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "sqlite-final", athSqliteFinal, FICL_WORD_DEFAULT);
#endif

#if defined(SYSV_IPC)
    ficlDictionarySetPrimitive(dictionary, (char *)"openqueue", athOpenQueue, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"closequeue", athRmQueue, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"msg-send", athMsgSend, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"msg-recv", athMsgRecv, FICL_WORD_DEFAULT);
#endif
    /*
#ifdef LIST
ficlDictionarySetPrimitive(dictionary, "mklist", athMkList, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "rmlist", athRmList, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-len", athListLen, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-append-string", athListAppendString, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-append-int", athListAppendInt, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-get", athListGet, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-get", athListGet, FICL_WORD_DEFAULT);
ficlDictionarySetPrimitive(dictionary, "list-display", athListDisplay, FICL_WORD_DEFAULT);

#endif
*/
#ifdef MTHREAD
    //	ficlDictionarySetPrimitive(dictionary, "create-vm", athCreateVM, FICL_WORD_DEFAULT);
    //	ficlDictionarySetPrimitive(dictionary, "create-thread", athCreateThread, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "delete-thread", athDeleteThread, FICL_WORD_DEFAULT);
    //	ficlDictionarySetPrimitive(dictionary, "yield-thread", athYieldThread, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "create-mutex", athCreateMutex, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "lock-mutex", athLockMutex, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "trylock-mutex", athTryLockMutex, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "unlock-mutex", athUnlockMutex, FICL_WORD_DEFAULT);
    /*
       ficlDictionarySetPrimitive(dictionary, "mkqueue", athMkQueue, FICL_WORD_DEFAULT);
       ficlDictionarySetPrimitive(dictionary, "qput", athQput, FICL_WORD_DEFAULT);
       ficlDictionarySetPrimitive(dictionary, "qget", athQget, FICL_WORD_DEFAULT);
       ficlDictionarySetPrimitive(dictionary, "qempty?", athQempty, FICL_WORD_DEFAULT);
       ficlDictionarySetPrimitive(dictionary, "qfull?", athQfull, FICL_WORD_DEFAULT);
       ficlDictionarySetPrimitive(dictionary, "qsize", athQsize, FICL_WORD_DEFAULT);
       */   
#endif

#ifdef SOCKET
    ficlDictionarySetPrimitive(dictionary, (char *)"socket", athSocket, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-bind", athBind, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-listen", athListen, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-accept", athAccept, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-recv", athRecv, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-send", athSend, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"socket-connect", athConnect, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"socket-close", athClose, FICL_WORD_DEFAULT);
#endif
    ficlDictionarySetPrimitive(dictionary, (char *)"fd@", athFdGet, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"fcntl", athFcntl, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"seal", athSeal, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"unseal", athUnseal, FICL_WORD_DEFAULT);
#ifdef LIBMODBUS
    ficlDictionarySetPrimitive(dictionary, "modbus-new-rtu", athModBusNew, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "modbus-new-net", athModBusNewNet, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "modbus-connect", athModBusConnect, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "modbus-set-slave", athModBusSetSlave, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "modbus-get-timeout", athModBusGetTimeout, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "modbus-Set-timeout", athModBusSetTimeout, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "modbus-read-input-bits", athModBusReadInputBits, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "modbus-read-registers", athModBusReadRegisters, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "modbus-read-input-registers", athModBusReadInputRegisters, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "modbus-write-coil", athModbusWriteSingleBit, FICL_WORD_DEFAULT);


    ficlDictionarySetPrimitive(dictionary, "modbus-write-register", athModBusWriteRegister, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "modbus-write-multiple-registers", athModBusWriteMultipleRegisters, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "modbus-error-recovery", athModBusErrorRecovery, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "modbus-debug", athModBusDebug, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, "modbus-perror", athModBusPrintError, FICL_WORD_DEFAULT);


    ficlDictionarySetPrimitive(dictionary, "modbus-close", athModBusClose, FICL_WORD_DEFAULT);


#endif
#ifdef MODBUS
    ficlDictionarySetPrimitive(dictionary, (char *)"modbus-calc-crc", athCalcCRC, FICL_WORD_DEFAULT);
#endif

#ifdef SPREAD
    //    ficlDictionarySetPrimitive(dictionary, "test", athTest, FICL_WORD_DEFAULT); 
    ficlDictionarySetPrimitive(dictionary, (char *)"sp-connect", athSPConnect, FICL_WORD_DEFAULT);   
    ficlDictionarySetPrimitive(dictionary, (char *)"sp-join", athSPJoin, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"sp-leave", athSPLeave, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"sp-send", athSPSend, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"sp-recv", athSPRecv, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"sp-poll", athSPPoll, FICL_WORD_DEFAULT);
#endif

#ifndef FICL_ANSI
    ficlDictionarySetPrimitive(dictionary, (char *)"clock", ficlPrimitiveClock, FICL_WORD_DEFAULT);
    ficlDictionarySetConstant(dictionary,  (char *)"clocks/sec", CLOCKS_PER_SEC);
#ifndef EMBEDDED
    ficlDictionarySetPrimitive(dictionary, (char *)"pwd", ficlPrimitiveGetCwd, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"cd", ficlPrimitiveChDir, FICL_WORD_DEFAULT);
#endif
#endif				/* FICL_ANSI */
#if FICL_WANT_STRING
#warning "Defining string primitives"
    ficlDictionarySetPrimitive(dictionary, "spush", athStringPush, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "spop", athStringPop, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "s+", athStringJoin, FICL_WORD_DEFAULT);

#endif
#ifdef I2C
    ficlDictionarySetPrimitive(dictionary, "i2c-open", athI2cOpen, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "i2c-close", athI2cClose, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "i2c-write", athI2cWrite, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"i2c-read", athI2cRead, FICL_WORD_DEFAULT);
#endif    
#ifdef LINUX
    ficlDictionarySetPrimitive(dictionary, (char *)"ioctl", athIoctl, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"mmap", athMmap, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, "ticks",      athPrimitiveTick, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"start-clock", athStartTimer , FICL_WORD_DEFAULT);
#endif
#ifdef SERIAL
    ficlDictionarySetPrimitive(dictionary, (char *)"open-serial", athOpenSerialPort, FICL_WORD_DEFAULT);
#endif

#ifdef REDIS
    ficlDictionarySetPrimitive(dictionary, (char *)"redis-connect", athConnectRedis, FICL_WORD_DEFAULT);
    //    ficlDictionarySetPrimitive(dictionary, "redis-connect-local", athLocalConnectRedis, FICL_WORD_DEFAULT);

    //    ficlDictionarySetPrimitive(dictionary, "redis-disconnect", athConnectRedis, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"redis-ping", athRedisPing, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"redis-cmd", athRedisCmd, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"redis-get-string", athRedisGetString, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"redis-get-type", athRedisGetType, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"redis-free-reply", athRedisFreeReply, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"redis-get-elements", athRedisGetElements, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"redis-get-element", athRedisGetElement, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"redis-get-integer", athRedisGetInteger, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)"redis-context-get-err", athRedisContextErr, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"redis-context-get-errmsg", athRedisContextErrMsg, FICL_WORD_DEFAULT);
    ficlDictionarySetPrimitive(dictionary, (char *)"redis-context-get-flags", athRedisContextFlags, FICL_WORD_DEFAULT);

    ficlDictionarySetPrimitive(dictionary, (char *)".redis-reply", athRedisDisplayReply, FICL_WORD_DEFAULT);
#endif
    return;
}
