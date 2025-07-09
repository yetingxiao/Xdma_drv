
TARGET=do_config

rm -rf  ${TARGET}
#gcc -o xdma  *.c  -Drtpc_debug
#gcc -o ${TARGET}  *.c  -Dvi53xx_debug -L ./ -lrt -lvi53xx
gcc -o ${TARGET} -I /opt/etas/include -L /opt/etas/lib/  *.c -lrt -lvi53xx 
