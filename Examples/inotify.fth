load lib.fth
load struct.fth

0X01 constant _SYS_INOTIFY_H
0X01 constant IN_ACCESS
0X02 constant IN_MODIFY
0X04 constant IN_ATTRIB
0X08 constant IN_CLOSE_WRITE
0X10 constant IN_CLOSE_NOWRITE
0X18 constant IN_CLOSE
0X20 constant IN_OPEN
0X40 constant IN_MOVED_FROM
0X80 constant IN_MOVED_TO
0XC0 constant IN_MOVE
0X100 constant IN_CREATE
0X200 constant IN_DELETE
0X400 constant IN_DELETE_SELF
0X800 constant IN_MOVE_SELF
0X2000 constant IN_UNMOUNT
0X4000 constant IN_Q_OVERFLOW
0X8000 constant IN_IGNORED
0X18 constant IN_CLOSE
0XC0 constant IN_MOVE
0X1000000 constant IN_ONLYDIR
0X2000000 constant IN_DONT_FOLLOW
0X4000000 constant IN_EXCL_UNLINK
0X20000000 constant IN_MASK_ADD
0X40000000 constant IN_ISDIR
0X80000000 constant IN_ONESHOT
0XFFF constant IN_ALL_EVENTS

0xff constant NAME_MAX



-1 value fd
-1 value wd
-1 value buffer
255 constant /buffer

struct
  (int) chars field inotify-wd
  (int) chars field inotify-mask
  (int) chars field inotify-cookie
  (int) chars field inotify-len      \ name length
  NAME_MAX chars field inotify-name  \ name of file
endstruct /inotify-event

\  struct inotify_event
\  {
\    int wd;       /* Watch descriptor.  */
\    uint32_t mask;    /* Watch mask.  */
\    uint32_t cookie;  /* Cookie to synchronize two events.  */
\    uint32_t len;     /* Length (including NULs) of name.  */
\    char name __flexarr;  /* Name.  */
\  };

: init
    inotify-init abort" inotify-init failed" to fd

\   wd = inotify_add_watch( fd, "/tmp", IN_CREATE | IN_DELETE );

\    fd s" /tmp" IN_CREATE IN_DELETE or inotify-add-watch abort" inotify-add-watch" to wd
    fd s" /tmp" IN_CREATE inotify-add-watch abort" inotify-add-watch" to wd

    /inotify-event allocate abort" allocate failed" to buffer
    buffer /inotify-event erase
    fd buffer /inotify-event read
    buffer /inotify-event dump

\    fd wd inotify-rm-watch abort" inotify-rm-watch failed." 
;

init

