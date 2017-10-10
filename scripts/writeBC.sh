#!/bin/bash

DIR=md5-`echo $2 | md5sum | cut -d' ' -f 1`
mkdir -p ${DIR}

amtf=`printf "%f" $2`
if ((`echo "$amtf > 100" | bc -l` )); then    
    echo $2 >  ${DIR}/$1
    echo $3 > "${DIR}/evm.bc"
    echo "$2 - $1"
fi
