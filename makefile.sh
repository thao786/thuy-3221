gcc -c mapping.c 
gcc -shared -o mapping.so mapping.c 

gcc -c reading.c 
gcc -shared -o reading.so reading.c 

gcc -o nlcnt nlcnt.c -ldl

./nlcnt