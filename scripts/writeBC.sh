DIR=md5-`echo $2 | md5sum | cut -d' ' -f 1`
mkdir -p ${DIR}

touch ${DIR}/$1
echo $2 > "${DIR}/evm.bc"
