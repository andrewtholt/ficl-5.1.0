# 
# No modbus, no ini file parser
# 
TARGET= -DLINUX -DDYNLIB -DFICL_WANT_LZ_SOFTCORE=0 -DPOSIX_IPC -DSYSV_IPC -DSOCKET -DCURSES

LIBS=-ldl -lncurses

