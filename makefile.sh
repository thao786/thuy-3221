gcc -shared -o mapping.so -fPIC mapping.c
gcc -shared -o reading.so -fPIC reading.c

gcc -o nlcnt nlcnt.c -ldl

./nlcnt