
only forth also oop definitions
-1 value database

load mydump.fth

object subclass record
    c-string obj: .key
    c-string obj: .data
    c-cell obj: .db

    : init { db  2:this -- }
        db this --> .db --> set
    ;

    : set-key { ptr len 2:this -- }
        ptr len
        this --> .key --> set
    ;

    : set-value { ptr len 2:this -- }
        ptr len
        this --> .data --> set
    ;

    : get-value { 2:this -- ptr len }
        this --> .data --> get
    ;

    : dump { 2:this -- }
        ." DB :" this --> .db --> get . cr
        ." Key:"
        this --> .key  --> type 
        ." :=:"
        this --> .data --> type
        ." :" cr
        this --> .db --> get gdbm-dump
    ;

    : (write-data) { 2:this }
        this --> .key  --> get
        this --> .data --> get
        this --> .db   --> get
    ;

    : (read-data) { 2:this }
        this --> .key  --> get
        this --> .db   --> get
    ;

    : insert { 2:this }
        this --> (write-data)
        gdbm-insert abort" INSERT"
    ;

    : update { 2:this }
        this --> (write-data)
        gdbm-update abort" UPDATE"
    ;

    : delete { 2:this }
        this --> (read-data)
        gdbm-delete abort" DEL"
    ;

    : fetch { 2:this }
        this --> (write-data)
        gdbm-fetch abort" FETCH"
    ;

    : clear { 2:this }
        this --> .key  --> clear
        this --> .data --> clear
    ;

end-class

s" /tmp/tst.db" r/w gdbm-open abort" OPEN" to database
database record --> new data

s" TEST" data --> set-key
s" DATA" data --> set-value

data --> get-value .s
data --> insert

s" NEW" data --> set-value
data --> update
data --> fetch

data --> dump

database gdbm-close


