
s" /tmp/config.fth" file-status nip 0= [if]
        .( loading config file ) cr
        include /tmp/config.fth
[else]
        .( Failed to load config file ) cr
        abort
[then]

load lib.fth
-1 value sock
-1 value sock2
-1 value unitid
\ -1 value semid
\ -1 value shmid
\ -1 value shm-ptr

0x04 constant length-offset
0x06 constant id-offset
0x07 constant function-offset
0x08 constant register-offset
0x0a constant register-count-offset

255 allocate abort" Allocate failed" value modbus-slave-ip-buffer 
255 allocate abort" Allocate failed" value modbus-slave-op-buffer 
255 allocate abort" Allocate failed" value scratch

: init
	modbus-slave-ip-buffer 255 erase
	modbus-slave-op-buffer 255 erase
	scratch 255 erase

	general_sem semtran to semid

\	general_shm general_regcnt 896 shmget to shmid
\	shmid shmat to shm-ptr

	scratch s" UNITID" getenv 0= if
		1 to unitid
	else
		scratch evaluate to unitid
	then
;

: display-packet { buf -- }
	cr
	buf length-offset + c@ 0x100 *
	buf length-offset 1+ + c@ 0xff and or
	." Msg len  : " . cr

	." Unit id  : " buf id-offset + c@ . cr
	." Function : " buf function-offset + c@ . cr

	buf register-offset + c@ 0x100 *
	buf register-offset 1+  + c@ 0xff and or
	." Register : " . cr

	buf register-count-offset + c@ 0x100 *
	buf register-count-offset 1+ + c@ 0xff and or
	." Count    : " . cr
;

: read-holding-registers { reg count -- fail_flag }
\
\ first check that address and count make sense for me.
\
	reg count + general_regcnt > if 
	\
	\ Attempt to read past the end of the registers.
	\
		-1 
	else
		modbus-slave-op-buffer 255 erase

		count 2* 3 + modbus-slave-op-buffer 5 + c!
		unitid modbus-slave-op-buffer 6 + c!
		0x03 modbus-slave-op-buffer 0x07 + c!
		count 2* modbus-slave-op-buffer 0x08 + c!

		semid getsem drop
		shm-ptr reg 2* + 
		modbus-slave-op-buffer 0x09 +
		count 2* move
		semid relsem drop

		cr modbus-slave-op-buffer 0x21 dump cr

		modbus-slave-op-buffer  count 2* 9 + sock2 socket-send drop
		0
	then
;

: write-multiple-registers { addr cnt -- }
	cr
	." Address    is " addr . cr
	." Byte count is " cnt . cr

	cnt 2* 0 do
		addr i + general_roreg > if

			semid getsem drop
			modbus-slave-ip-buffer i + 13 + c@
			shm-ptr addr 2* + i + c!
			semid relsem drop
		then

		modbus-slave-ip-buffer modbus-slave-op-buffer 0x0c move
		modbus-slave-op-buffer 0x0c sock2 socket-send drop
		hex cr modbus-slave-op-buffer 0x0c dump cr decimal
	loop
;

: execute-request { func reg count -- }
	\
	\ Class 0 are must have
	\ Class 1 are a good idea
	\
	func case
		 \ Class 1
		 1 of ." Read Coil Status" endof
		 \ Class 1
		 2 of ." Read Discrete Inputs" endof
		 3 of
		 	\ Class 0
		 	." Read Holding Registers" cr
			reg count read-holding-registers
			if
				." Problem with read"
			else
				." Read OK"
			then cr
		endof
		 \ Class 1
		 4 of ." Read Input Registers" endof
		 \ Class 1
		 5 of ." Force Single Coil" endof
		 \ Class 1
		 6 of ." Write Single Register" endof
		 \ Class 1
		 7 of ." Read Exception Status" endof
		16 of
		 	\ Class 0
			." Write Multiple Registers"
			reg count write-multiple-registers
		endof
		." Illegal or unimplemented function "
	endcase

;

: process-packet { buf -- }
	cr
	." My unit id: " unitid . cr

	 buf id-offset + c@ unitid = if
	 	." The message is for me" cr
		\
		\ Get function code
		\
		buf function-offset + c@
		\
		\ Register
		\
		buf register-offset + c@ 0x100 *
		buf register-offset 1+  + c@ 0xff and or
		\
		\ Count
		\
		buf register-count-offset + c@ 0x100 *
		buf register-count-offset 1+ + c@ 0xff and or
		execute-request
	else
		." The message is for somebody else" cr
	then
;

: modbus-slave
	init
	modbus-slave-ip-buffer 255 erase
	clr-errno
	socket to sock 
	
	502 sock socket-bind ." bind " if
		perror
		bye
	then

	sock socket-listen	." listen " . cr
	perror

	begin
		." modbus_slave: depth " depth . cr
		sock socket-accept ." accept " to sock2

		modbus-slave-ip-buffer 255 sock2 socket-recv ." recv " cr

		modbus-slave-ip-buffer swap dump
		." Stack depth is " depth . cr
	
		modbus-slave-ip-buffer display-packet
	
		modbus-slave-ip-buffer process-packet
	
		cr 
	again

	sock2 socket-close
\	sock socket-close
;

\  tst
