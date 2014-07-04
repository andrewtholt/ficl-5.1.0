include strings.fth

variable ptr
variable buffer

: main
    255 allocate abort" Allocate failed"
    buffer !

    buffer @ 255 erase

    s" World" buffer @ swap move

    s" This is a test" ptr $!

;

: tst { string len buff size }
    buff buff size + move
;

main


