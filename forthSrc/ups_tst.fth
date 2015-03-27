
255 allocate abort" Alloca failed" constant apc-buffer
apc-buffer 255 erase

-1 constant fd

s" /dev/ttyS0" r/w open-file abort" open tty failed" to fd
: ups-alive?                                                       
        14 signal                                                  
        5 alarm                                                    
        clr-errno                                                  
        s" Y" fd write-file perror abort" ups-alive?: write-failed"
        fd flush-file drop                                                
        apc-buffer 255 fd read-line drop 2drop    
        errno 0<> if
            ." No response from UPS" cr
            perror
        then
        0 alarm           
        errno 0=                                          
        clr-errno                                               
;                                                               
            

ups-alive?
apc-buffer 10 type

