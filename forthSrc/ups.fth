255 allocate abort" allocate failed" value apc-buffer
apc-buffer 255 erase
-1 value fd
\ -1 value ptr
\ -1 value semid
\ create-mutex value shm-mtx

300 constant apc-ok-sleep
10 constant apc-fail-sleep
variable cnt

apc-ok-sleep value apc-poll-time

\ s" /tmp/config.fth" file-status nip 0= [if]
\ 	.( loading config file ) cr
\  	include /tmp/config.fth
\ [else]
\	.( Failed to load config file ) cr
\	abort
\ [then]

\ : shm-w! ( n offset --- )
\ 
\ 	shm-mtx lock-mutex drop
\ 	semid getsem drop
\ 	ptr + apc_ups_offset + >r
\ 	dup
\ 	0x100 / r@ c!
\ 	0xff and r> 1+ c!
\ 	semid relsem drop
\ 	shm-mtx unlock-mutex drop
\ ;

: clr-shm
	shm-mtx lock-mutex drop
	semid getsem drop
	shm-ptr apc_ups_offset + apc_ups_ups-area-size 0xff fill 
	semid relsem drop
	shm-mtx unlock-mutex drop
;

: ups-init
	0 cnt !
	apc-buffer s" DEVICE" getenv ?dup 0= if
		." Environment variable DEVICE not set" cr
		apc_ups_device 2@ 
		." Opened device " apc_ups_device 2@ type cr
	else
		apc-buffer swap
	then
	r/w open-file abort" open-file failed" to fd 
	\
	\ NOTE: the third parameter (896) is Octal 1600, rw owner only and
	\ create the segment if it does not exist.
	\
\	general_shm general_regcnt 896 shmget shmat to ptr
\	general_sem semtran to semid
	clr-shm
;

: smart-mode
	s" Y" fd write-file abort" smart-mode: write-filed"
	fd flush-file drop
	apc-buffer 255 fd read-line abort" read-file failed" 2drop
;

: ups-alive?
	14 signal
	5 alarm
	clr-errno
	s" Y" fd write-file perror abort" ups-alive?: write-failed"
	fd flush-file drop
	apc-buffer 255 fd read-line drop 2drop
	0 alarm
	errno 0=
	clr-errno
;

: battery-voltage ( --- n )
	s" B" fd write-file abort" battery-voltage: write-filed"
	fd flush-file drop
	0 s>d
	apc-buffer 255 fd read-line abort" read-file failed" drop
	apc-buffer swap >number 2drop drop
	apc_ups_battery-voltage shm-w!
;

: run-time
        s" j" fd write-file abort" battery-voltage: write-filed"
   	fd flush-file drop
       	0 s>d
        apc-buffer 255 fd read-line abort" read-file failed" drop
       	apc-buffer swap >number 2drop drop
	apc_ups_runtime shm-w!
;

: line-hz
	s" F" fd write-file abort" line-hz: write-failed"
	fd flush-file drop
	0 s>d
	apc-buffer 255 fd read-line abort" read-file failed" drop
	apc-buffer swap >number 2drop drop
	apc_ups_line-hz shm-w!
;

: ip-line-voltage
	s" L" fd write-file abort" ip-line-voltage: write-failed"
	fd flush-file drop
	0 s>d
	apc-buffer 255 fd read-line abort" read-file failed" drop
	apc-buffer swap >number 2drop drop
	apc_ups_ip-line-voltage shm-w!
;

: max-line-voltage
	s" M" fd write-file abort" max-line-voltage: write-failed"
	fd flush-file drop
	0 s>d
	apc-buffer 255 fd read-line abort" read-file failed" drop
	apc-buffer swap >number 2drop drop
	apc_ups_max-line-voltage shm-w!
;

: min-line-voltage
	s" M" fd write-file abort" min-line-voltage: write-failed"
	fd flush-file drop
	0 s>d
	apc-buffer 255 fd read-line abort" read-file failed" drop
	apc-buffer swap >number 2drop drop
	apc_ups_min-line-voltage shm-w!
;

: op-line-voltage
	s" O" fd write-file abort" op-line-voltage: write-failed"
	fd flush-file drop
	0 s>d
	apc-buffer 255 fd read-line abort" read-file failed" drop
	apc-buffer swap >number 2drop drop
	apc_ups_op-voltage shm-w!
;

: power-load
	s" P" fd write-file abort" power-load: write-failed"
	fd flush-file drop
	0 s>d
	apc-buffer 255 fd read-line abort" read-file failed" drop
	apc-buffer swap >number 2drop drop
	apc_ups_power-load shm-w!
;

: charge
        s" f" fd write-file abort" battery-voltage: write-failed"
        fd flush-file drop
        0 s>d
        apc-buffer 255 fd read-line abort" read-file failed" drop
        apc-buffer swap >number 2drop drop
	apc_ups_battery-level shm-w!
;

: internal-temp
	s" C" fd write-file abort" battery-voltage: write-filed"
	fd flush-file drop
	apc-buffer 255 fd read-line abort" read-file failed" drop
	dup 3 > if
		0 s>d rot apc-buffer swap >number 2drop drop
		drop
	else
		drop 0xffff
	then
	apc_ups_int-temp shm-w!
;

: get-status
	s" Q" fd write-file abort" smart-mode: write-filed"
	fd flush-file drop
	apc-buffer 255 fd read-line abort" read-file failed" 2drop
	base @ >r
	hex
	0 s>d apc-buffer 2 >number 2drop drop
	r> base !
	apc_ups_status-flags shm-w!
;

: event
	apc-buffer c@
	case
		[char] ? of ." Dumb Mode" endof
		[char] $ of ." Online" 
				apc-ok-sleep to apc-poll-time
			endof
		[char] ! of ." On Battery"
				apc-fail-sleep to apc-poll-time
			endof
		[char] % of ." Battery Discharged" endof
				\ Touch a file in flash to indicate that power
				\ failed.
		[char] + of ." Battery charged" endof
		[char] * of ." Test" endof
		[char] | of ." EEPROM Value changed" endof
	endcase
	cr 
;

: main
fd -1 = abort" Device not open"
shm-ptr -1 = abort" Attach to shared memory failed"	
begin
	1 cnt +!
	cnt @ apc_ups_count shm-w!
	clr-errno
	ups-alive?
	if
		14 signal
		apc-poll-time alarm
		smart-mode
		." +++++++++++++++++++++++++++" cr
		." Loop Count is   "  cnt @ . cr
		." ---------------------------" cr
		." Battery Level   "  battery-voltage . ." Volts" cr
		." UPS Status      "  get-status . cr
		." Internal Temp   "  internal-temp dup 0xffff =
		if drop ." NA" else . then cr
		." Run Time        "  run-time . ." Minutes" cr 
		." Line Frequency  " line-hz . ." Hz" cr
		." Input           " ip-line-voltage . ." Volts" cr
		." Max Input       " max-line-voltage . ." Volts" cr
		." Min Input       " min-line-voltage . ." Volts" cr
		." Power Load      " power-load . ." %" cr
		." Output          " op-line-voltage . ." Volts" cr
		." Battery Charge  "  charge . ." %" cr

		apc-buffer 1 fd read-file nip
		4 = if ." Timed out" cr then
		event
	else
		." No response from ups" cr
		clr-shm
	then
	." Stack depth is " depth . cr
again
;

: tst
fd -1 = abort" Device not open"
shm-ptr -1 = abort" Attach to shared memory failed"	
	
begin
	1 cnt +!
	cnt @ apc_ups_count shm-w!
	clr-errno
	ups-alive?
	if
		14 signal
		apc-poll-time alarm
		smart-mode
		battery-voltage
		get-status
		internal-temp 
		run-time
		line-hz
		ip-line-voltage
		max-line-voltage
		min-line-voltage
		power-load
		op-line-voltage
		charge

		apc-buffer 1 fd read-file nip
		4 = if ." Timed out" cr then
		event
	else
		." No response from ups" cr
		clr-shm
	then
	." ups: depth is " depth . cr
	depth 0> if
		.s
	then
again
;

: start
	ups-init
\	main
	tst
;

: t1
	begin 1 shm-ptr 28 + +! 1000 ms again 
;

\ create-vm value ups-vm
\ create-vm value tst-vm
\ s" start" ups-vm create-thread . cr
\ s" t1" tst-vm create-thread . cr
\ 
\ 
