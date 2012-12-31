#!/bin/sh

ARCH=${ARCH:-'32'}

for i in `seq 10 14`; do
	j=`echo $i + 1 | bc`
	make main -B CFLAGS="-m$ARCH -DEBP_ADDR_IDX=$i -DRET_ADDR_IDX=$j"
	echo Test with ebp at offset $i
	./main
done

