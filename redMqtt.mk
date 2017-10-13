# 
# Redis & MQTT
# 
# TARGET= -DLINUX -DDYNLIB -DFICL_WANT_LZ_SOFTCORE=0 -DMQTT
TARGET=-DREDIS -DFICL_WANT_STRING -DLINUX -DDYNLIB -DFICL_WANT_LZ_SOFTCORE=0 -DPOSIX_IPC -DSYSV_IPC -DSOCKET -DINIPARSER -DMQTT
 

LIBS=-ldl -lmosquitto

# 
# Redis plus ini parser
# 

LIBS+=-liniparser -lhiredis
INCLUDES=-I /usr/include/hiredis -I /usr/local/include

