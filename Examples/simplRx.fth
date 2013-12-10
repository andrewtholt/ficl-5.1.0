

load mydump.fth

1024 allocate abort" allocate" value buffer

variable sender
variable out-data
variable in-data


: tst
    buffer sender !

	2 out-data !
	-1 in-data !
	
	s" rx" simpl-attach abort" ATTACH"
	
    begin
		buffer in-data 4 simpl-receive abort" RX"
		." RX length:" . cr
		
		in-data @ 1+ out-data !
		
		buffer @ dup strlen out-data @ 4 simpl-reply abort" REPLY"
		." REPLY length:" . cr

        in-data @ 10 =
    until
    .s
; 

tst

