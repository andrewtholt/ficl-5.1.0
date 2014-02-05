-1 value shmid
-1 value semid
-1 value ptr
-1 value shm-size
-1 value my-id

896 constant OWN_RW  \ 0600 Octal, permissions

1024 to shm-size

999 shm-size OWN_RW shmget to shmid

shmid shmat to ptr

shmid . cr

s" This is a test" ptr swap move

ptr 128 dump

