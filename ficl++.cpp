
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "ficl.h"
#include "cstring.h"

#include <termios.h>


int main() {
    int returnValue = 0;
    char buffer[255];


    ficlSystem *fSystem;
    ficlVm *vm;

    fSystem = ficlSystemCreate(NULL);
    vm = ficlSystemCreateVm(fSystem);

    while (returnValue != FICL_VM_STATUS_USER_EXIT) {
        fputs(FICL_PROMPT, stdout);
        fflush(stdout);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
        returnValue = ficlVmEvaluate(vm, buffer);
    }   

    ficlSystemDestroy(fSystem);
    return 0;

}

