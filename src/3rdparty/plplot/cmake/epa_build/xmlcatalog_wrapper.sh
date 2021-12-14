#!/bin/bash
# Wrapper for xmlcatalog command to workaround entity mangling

if [ "$#" == "2" -a -f $1 ]; then
    xmlcatalog --shell $1 <<EOF
public "$2"
system "$2"
exit
EOF
echo $?
else
    xmlcatalog $*
fi
