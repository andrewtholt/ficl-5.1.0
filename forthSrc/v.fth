load lib.fth

255 allocate abort" Failed to allocate query buffer" constant buffer
buffer 255 erase

255 allocate abort" Failed to allocate query buffer" constant query
query 255 erase

255 allocate abort" Failed to allocate scratch buffer" constant scratch
scratch 255 erase

32 allocate abort" Failed to allocate name buffer" constant iam
iam 32 erase

-1 value sqldb
-1 value sqlite-vm

: init
    s" /tmp/tst.db" sqlite-open
    ?dup if
        to sqldb
    else
        ." Database does not exist" cr
        abort
    then
;

init
vocabulary testing
only forth also testing definitions

: bye bye ;

: cr cr ;

: words words ;

: seal seal ;

: help
    cr
    ." Complete help is not yet available" cr
    cr
    ." version" 09 emit ." : Display the current version" cr
    ." my-name" 09 emit ." : Display the client name" cr
    ." bye" 09 emit ." : Disconnect." cr
    
    cr
;

: version
    ." 0.01_TEST" cr
;

: my-name
    iam c@ 0= if
        s" NOT_SET"
    else
        iam count
    then
    type cr
;

: test
	iam 32 erase
	bl word dup c@ 0x7f and 1+ iam swap move
	iam count type cr
	
	s" select count(*) from client where name = '%s';" query swap move
	
	cr ." Query" cr
	query 64 mdump

	iam count query dup strlen scratch 255 sprintf 0= abort" sprintf: failed"
	sqldb sqlite-compile abort" sqlite-compile failed" to sqlite-vm

    iam count type cr
	
	sqlite-vm buffer sqlite-fetch
\	buffer c@ 0x30 = if
\	    ." No entry for this client" cr
\	    s" insert into client ( name,status ) values ( '%s','CONN');"
\        query swap move
\
\        query dup strlen type cr
\	    buffer count query dup strlen scratch 255
\        sprintf 0= abort" sprintf: failed"
\	else
\	    ." Entry for this client"
\	then
\	cr
;

: pwd pwd ;

: uname uname ;

only testing
seal

