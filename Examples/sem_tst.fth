
-1 value semid


: init
    10 semtran to semid
    0 semid setsemvalue 0= abort" setsemvalue"
;

: test
    semid -1 = if
        ." Running init" cr
        init
    then

    ." Semvalue is:"
    semid getsemvalue . cr

    ." A sem value of zero means locked, so release it:" 
    semid relsem .
    cr
    semid getsemvalue . cr

    ." and now getsem wont block." cr
    semid getsem .
;

: clean-up
    semid 0> if
        semid rmsem abort" rmsemid"
    then
;

test

