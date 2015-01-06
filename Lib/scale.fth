
\ int scale(unsigned int val, unsigned int from_min, unsigned int from_max, int to_min, int to_max) 

\ ret = (val * ( to_max - to_min)) / ( from_max - from_min );

: scale ( val from_min from_max to_min to_max )
    swap -          \ val from_min from_max to_diff
    -rot swap -     \ val to_diff from_diff
    */
;

\ 
\ Example of usage.
\
\ : tst ( val - ret )
\     0 1024 0 255 scale
\ ;
\ 
