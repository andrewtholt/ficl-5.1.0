

load mydump.fth
-1 value rx
variable out-data
variable in-data

2 out-data !
-1 in-data !

s" tx" simpl-attach abort" ATTACH"

s" rx" simpl-locate abort" LOCATE" to rx


rx out-data 4 in-data 4 simpl-send

in-data @ . cr

.s

simpl-detach
