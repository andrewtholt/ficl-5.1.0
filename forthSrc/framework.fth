
\ -1 value sock
\ -1 value sock2
\ -1 value unitid
-1 value semid
-1 value shmid
-1 value shm-ptr
create-mutex value shm-mtx

\ 0x04 constant length-offset
\ 0x06 constant id-offset
\ 0x07 constant function-offset
\ 0x08 constant register-offset
\ 0x0a constant register-count-offset

\ 255 allocate abort" Allocate failed" value ip-buffer 
\ 255 allocate abort" Allocate failed" value op-buffer 
255 allocate abort" Allocate failed" value framework-buffer
framework-buffer 255 erase

\ join string 2 to the end of string 1.
\ There must be sufficient space at str1 fro the new string.
\
: cat { str1 len1 str2 len2 -- len3 }               
        str2 str1 len1 + len2 move
        str1 len1 len2 +
;
 

: $load { addr len -- }
	addr dup 5 + len move
	s" load " addr swap move
	addr len 5 + evaluate
;

s" /tmp/config.fth" file-status nip 0= [if]
        .( loading config file ) cr
        include /tmp/config.fth
[else]
        .( Failed to load config file ) cr
        abort
[then]

framework-buffer s" FICLLIBDIR" getenv
?dup 0= [if]
	.( Environment variable FICLLIBDIR not set, aborting ) cr
	abort
[then]

framework-buffer swap s" /lib.fth" cat $load

: shm-w! ( n offset --- )

	shm-mtx lock-mutex drop
	semid getsem drop
	shm-ptr + apc_ups_offset + >r
	dup
	0x100 / r@ c!
	0xff and r> 1+ c!
	semid relsem drop
	shm-mtx unlock-mutex drop
;

: shm-w@ ( offset -- w )
	shm-mtx lock-mutex drop
	semid getsem drop
	shm-ptr + apc_ups_offset + dup
	c@ 0x100 * swap 1+ c@ 0xff and or
	semid relsem drop
	shm-mtx unlock-mutex drop
;

\ load lib.fth:1
general_shm general_regcnt 896 shmget shmat to shm-ptr
general_sem semtran to semid
1 semid setsemvalue


load ups.fth
load modbus_slave.fth

.( Starting Threads ) cr
create-vm value ups-vm
create-vm value tst-vm
create-vm value modbus-slave-vm
s" start" ups-vm create-thread . cr
1000 ms
\ s" t1" tst-vm create-thread . cr
\ 1000 ms
s" modbus-slave" modbus-slave-vm create-thread . cr

t1
