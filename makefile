all: nlcnt

mapping.so: mapping.c
	gcc -shared -o mapping.so -fPIC mapping.c

reading.so: reading.c
	gcc -shared -o reading.so -fPIC reading.c

nlcnt: nlcnt.c
	gcc -o nlcnt nlcnt.c -ldl

clean:
	rm *o nlcnt