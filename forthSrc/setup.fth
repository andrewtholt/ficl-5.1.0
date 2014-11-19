\
\ Run this before anything else is started.
\ It :
\ creates the semaphore, then sets it to zero (i.e. aquired)
\ creates the shared memory segment, and erases it to 0
\ release the semaphore.
\ exits
\
-1 value semid
-1 value shmid
-1 value ptr

load lib.fth

s" /tmp/config.fth" file-status nip 0= [if]
	.( loading config file ) cr
	include /tmp/config.fth
[else]
	.( Failed to load config file ) cr
	abort
[then]

: setup-sem
	general_sem semtran to semid
	cr
	." Semaphore id is " semid . cr
\
\ Block any body from attempting to access shared memory whilst starting
\
	0 semid setsemvalue
;

: setup
	setup-sem
\
\ Create shared memory
\
	general_shm general_regcnt 896 shmget to shmid
	shmid shmat to ptr
	ptr general_regcnt erase
\
\ Release the semaphore.
\
	semid relsem
;

setup
bye
