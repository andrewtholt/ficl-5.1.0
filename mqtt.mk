# 
# No modbus, no ini file parser
# 
TARGET= -DLINUX -DDYNLIB -DFICL_WANT_LZ_SOFTCORE=0 -DMQTT

LIBS=-ldl -lmosquitto

