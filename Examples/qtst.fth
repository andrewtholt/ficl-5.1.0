load mydump.fth

-1 value buffer
create-vm value new-vm1
create-vm value new-vm2

255 allocate abort" alloc" to buffer

10 mkqueue value Q

: producer
    Q s" hello" qput
    Q s" there" qput
    100 sleep
.s
;

: consumer
    Q buffer qget
    buffer 10 mdump

    Q buffer qget
    buffer 10 mdump
;

s" consumer" new-vm1 create-thread drop
s" producer" new-vm2 create-thread drop
