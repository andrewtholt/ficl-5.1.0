\ include lib.fth

.( tst.fth ) cr

-1 value fd

create buffer 128 allot

: tst
  s" /tmp/tst.txt" r/w open-file if 
    drop s" /tmp/tst.txt" r/w create-file abort" Failed to create file"
  then
  to fd	
  
  s" this was a test" fd write-file
  0x0a buffer c!
  buffer 1 fd write-file
  
  fd close-file
;

