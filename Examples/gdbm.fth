load mydump.fth

-1 value db
-1 value key

( Open * create a db )
s" /tmp/tst.db" r/w 512 gdbm-open
abort" DB failed"

( save database ptr & key area )
to db
to key

512 allocate abort" allocate" value val

s" TEST" key swap zmove

key 32 mdump

key dup strlen s" DATA" db gdbm-insert abort" Insert"

key dup strlen s" NEW" db gdbm-update abort" Update"

s" SECOND" key swap zmove
key dup strlen s" TESTING" db gdbm-insert abort" Insert"

key dup strlen val db gdbm-fetch .
val 32 mdump

db gdbm-sync

db gdbm-dump
key dup strlen db gdbm-delete 
db gdbm-reorg . cr cr

( close db & release key area )
key db gdbm-close

.s
