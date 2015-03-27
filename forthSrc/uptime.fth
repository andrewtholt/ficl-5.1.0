\ ficl

-1 value fd
-1 value buffer
0 s>d 2value tmp

: init
	255 allocate abort" Allocate failed" to buffer
	buffer 255 erase
;

: main { | t l  -- }
	init
	s" /proc/uptime" r/o open-file abort" no file" to fd
	
	buffer 255 fd read-line abort" read failed" drop
	>r
	[char] . buffer r> strtok 
	>r
	to l to t
	r>
	1- 2* 0 do drop loop
	0 s>d t l  >number drop nip nip

	fd close-file drop
;

main
.( Uptime is ) .
.(  Seconds ) cr

bye
