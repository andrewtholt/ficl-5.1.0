# 
# No modbus, no ini file parser
# 
TARGET= -DLINUX -DDYNLIB -DFICL_WANT_LZ_SOFTCORE=0 -DMTHREAD -DTIMERS

LIBS=-ldl -L/usr/local/lib -lathtimers
INC=-u/usr/local/include

