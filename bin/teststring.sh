#!/bin/sh
#

DEST="/home/cube/archive/000010/EB1/DATA"

#MOD=${DEST:19:3}
#MODID=${DEST:23}

# Not perfect, might be improved later
dir=${DEST#*/archive/} ; MODNO=${dir%%/*} ; 
dir=${DEST#*/$MODNO/} ; MODID=${dir%%/*}

shopt -s extglob
MODNO=${MODNO##+(0)}
echo "DEST=$DEST"
echo "MODNO=$MODNO"
echo "MODID=$MODID"


