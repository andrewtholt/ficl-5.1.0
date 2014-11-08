/*
** Ficl softcore
** both uncompressed and Lempel-Ziv compressed versions.
**
** Generated 2014/11/08 15:23:39
**/

#include "ficl.h"


static size_t ficlSoftcoreUncompressedSize = 1; /* not including trailing null */

#if !FICL_WANT_LZ_SOFTCORE

static char ficlSoftcoreUncompressed[] =
;

#else /* !FICL_WANT_LZ_SOFTCORE */

static unsigned char ficlSoftcoreCompressed[4] = {
	0x09, 0x01, 0x05, 0x00, 
	};

#endif /* !FICL_WANT_LZ_SOFTCORE */


void ficlSystemCompileSoftCore(ficlSystem *system)
{
    ficlVm *vm = system->vmList;
    int returnValue;
    ficlCell oldSourceID = vm->sourceId;
    ficlString s;
#if FICL_WANT_LZ_SOFTCORE
    char *ficlSoftcoreUncompressed = NULL;
    size_t gotUncompressedSize = 0;
    returnValue = ficlLzUncompress(ficlSoftcoreCompressed, (unsigned char **)&ficlSoftcoreUncompressed, &gotUncompressedSize);
    FICL_VM_ASSERT(vm, returnValue == 0);
    FICL_VM_ASSERT(vm, gotUncompressedSize == ficlSoftcoreUncompressedSize);
#endif /* FICL_WANT_LZ_SOFTCORE */
    vm->sourceId.i = -1;
    FICL_STRING_SET_POINTER(s, (char *)(ficlSoftcoreUncompressed));
    FICL_STRING_SET_LENGTH(s, ficlSoftcoreUncompressedSize);
    returnValue = ficlVmExecuteString(vm, s);
    vm->sourceId = oldSourceID;
#if FICL_WANT_LZ_SOFTCORE
    free(ficlSoftcoreUncompressed);
#endif /* FICL_WANT_LZ_SOFTCORE */
    FICL_VM_ASSERT(vm, returnValue != FICL_VM_STATUS_ERROR_EXIT);
    return;
}

/* end-of-file */
