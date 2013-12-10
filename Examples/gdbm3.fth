load mydump.fth

-1 value key
-1 value val
-1 value handle

32 allocate abort" allocate key" to key
32 allocate abort" allocate val" to val

s" /tmp/tst.db" r/w gdbm-open abort" gdbm-open" to handle

s" FOUR" key swap move
s" 4" val swap move

key dup strlen val dup strlen handle .s gdbm-insert

key dup strlen  type cr
val dup strlen  type cr

.( FETCH) cr
s" ONE" val 32 handle gdbm-fetch cr .s cr
.( DONE) cr

handle gdbm-dump

handle gdbm-close

