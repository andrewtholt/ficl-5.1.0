#!/usr/bin/fish

function help
    echo "Help ...."
    printf "\tPass in the version number as the sole argumenmt.\n"
end

cp ficl.h ficl.h.bak


set NAME (find . -name control)

if [ (count $argv) -ne 1 ]  
    help
    exit 1
end

set -x VER $argv[1]


set CMD "s/#define FICL_VERSION .*/#define FICL_VERSION \"$VER\"/"

sed $CMD < ficl.h.bak > ficl.h

for FILE in $NAME
    echo $FILE
    cp $FILE $FILE.bak
    set CMD "s/Version:.*/Version: $VER/"

    cat $FILE.bak | sed $CMD > $FILE
end

printf "\n\tNow do the following\n"
printf "\t\t./build.sh -a clean\n"
printf "\t\t./build.sh\n"
printf "\t\t./ficl -c bye\n\n"

