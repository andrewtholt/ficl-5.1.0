
: csi
    27 emit [char] [ emit
;

: cls
    csi 1 [char] J emit
;

: show-cursor
    csi ." ?P25h"
;

