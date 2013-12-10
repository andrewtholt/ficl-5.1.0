
0 value task-id
0 value task-id1

: tst
    begin
        task-yield
        1000 ms
        ." task 1" cr
    again
;

: tst1
    begin
        task-yield
        1000 ms
        ." task 2" cr
    again
;

tasks-enable

create-vm
' tst
task-create 0= abort" Task create failed." to task-id

create-vm
' tst1
task-create 0= abort" Task create failed." to task-id1

tasks-list
.s
task-id task-resume
task-id1 task-resume

tasks-list
\ tasks-disable
\ bye
