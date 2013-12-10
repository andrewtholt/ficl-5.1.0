load struct.fth

load mydump.fth

-1 value db
-1 value key
-1 value data

struct
512 chars field key
1 cells field key-len
512 chars field data
1 cells field data-len
endstruct record

record allocate abort" record allocate failed" value record-buffer

record-buffer record erase

s" /tmp/tst.db" r/w gdbm-open abort" gdbm-open" to db

s" TEST" record-buffer key drop swap zmove
s" DATA" record-buffer data  drop swap zmove


.s

: (string)
    drop dup strlen
;

record-buffer key  (string) record-buffer data (string) db gdbm-insert abort" gdbm-insert"

s" TEST1" record-buffer key drop swap zmove
record-buffer key  (string) record-buffer data (string) db gdbm-insert abort" gdbm-insert"
db gdbm-dump

s" L" record-buffer data  drop swap zmove
record-buffer key  (string) record-buffer data (string) db gdbm-update 

record-buffer data erase

record-buffer key  (string) record-buffer data (string) db gdbm-fetch 

record-buffer key (string) db gdbm-delete


db gdbm-dump
\ record-buffer key mdump
\ record-buffer data mdump
.s
