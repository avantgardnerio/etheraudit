cat $1 | sort -n --field-separator=',' --key=6 --reverse | cut -d , -f 3,7,6 --output-delimiter=' ' | head -n$2 | xargs -n 3 ./writeBC.sh
