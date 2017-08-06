# 
# Redis plus ini parser
# 
TARGET=-DREDIS -DFICL_WANT_STRING -DLINUX -DDYNLIB -DFICL_WANT_LZ_SOFTCORE=0 -DPOSIX_IPC -DSYSV_IPC -DSOCKET -DINIPARSER

LIBS=-ldl -liniparser -lhiredis
INCLUDES=-I /usr/include/hiredis

