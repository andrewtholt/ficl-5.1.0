255 allocate abort" Allocate failed" value scratch-buffer
scratch-buffer 255 erase

scratch-buffer s" M2MLIBDIR" getenv 
?dup 0= abort" Environment variable M2MLIBDIR not set.  Goodbye"

