#!/bin/bash

url="https://api.etherscan.io/api?module=contract&action=getabi&address=%s&apikey=%s"
key="8YIWC3X98HS61M7NUH4K3A5HYUEFQHJJQA"

for dir in md5-*; do
    for file in $dir/0x*; do
	amt=`cat $file`
	amtf=`printf "%f" $amt`
	if ((`echo "$amtf > 100" | bc -l` )); then
	    contractFile=${file##*/}
	    contractId=${contractFile%.*}
	    echo $contractId
	    if [ ! -f $dir/contractId.abi ]; then
		echo "Getting $file - $amt - '$dir/contractId.abi'"		
		abiurl=`printf $url $contractId $key`
		echo $abiurl
		wget "$abiurl" --output-document="$dir/contractId.abi"
		sleep 1
	    fi
	fi
    done
done
