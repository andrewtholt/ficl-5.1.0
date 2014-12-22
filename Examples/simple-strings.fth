


s" Hello," strsave 2value first
s" world!" strsave 2value second


: join { addr len addr1 len1 | addr2 len2 -- addr2 len2 }
    len len1 + dup allocate abort" Allocate failed"  to addr2 to len2

    addr addr2 len move
    addr1 addr2 len + len1 move


    addr2 len2
;

: delete ( addr len -- )
    2dup erase
    drop free abort" Free failed."
;

: grow { addr len add | na nl -- addr len }

    add len + to nl

    nl allocate abort" Allocate failed." to na

    na nl erase

    addr na len move
    na nl
;


: append-char ( c addr len -- )
    + c!
;

: append-cr ( addr len -- )
    0x0a -rot append-char
;

: place-char ( c idx  addr -- )
    swap append-char
;





