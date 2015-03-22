load lib.fth

-1 value sqldb
-1 value buffer
-1 value sqlite-vm

255 allocate abort" allocate failed" to buffer
buffer 255 erase

: tst
s" /tmp/tst" sqlite-open
?dup if to sqldb else abort then

\ s" select val from config where class = 'GENERAL' and key = 'SHM';"
\ s" select * from config where class = 'GENERAL' and key = 'SHM';"
s" select * from config;"
\ s" select count(*) from config;"
sqldb sqlite-compile abort" sqlite-compile failed" to sqlite-vm

begin
  sqlite-vm buffer sqlite-fetch
while
  buffer swap type cr
repeat
;

tst

