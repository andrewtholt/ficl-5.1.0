( Nicely formatted memory dumper for ficl )

\ Uncomment these for testing
\
\ 255 allocate abort" Allocation failed" value buffer
\ 0xff buffer c!
\ 0x30 buffer 1+ c!

: range ( addr cnt -- addr addr+cnt )
	over + 
;

: mdump-hex-line ( addr -- )
	base @ >r
	hex
	dup s>d <# # # # # # # # # #> type
	." :"
	0x10 range swap do 
		i c@ s>d <# # # #> type space
	loop
	r> base !
;

: mdump-ascii-line ( addr -- )
	0x10 range swap do 
		bl 0x80 i c@ within if i c@ emit else ." ." then
	loop
;

: mdump { addr cnt -- }
	addr cnt range swap do
		i mdump-hex-line ." :"
		i mdump-ascii-line cr
	0x10 +loop
;

\ Tests and examples 
\ buffer 20 mdump
\ buffer mdump-ascii-line
