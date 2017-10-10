DIR=md5-`echo $2 | md5sum | cut -d' ' -f 1`
mkdir -p ${DIR}

echo $2 >  ${DIR}/$1
echo $3 > "${DIR}/evm.bc"
echo $2
