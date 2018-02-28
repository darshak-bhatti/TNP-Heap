all: hello_bm 

hello_bm: hello_bm.c 
	$(CC) -g -O0 hello_bm.c -o hello_bm -I/usr/local/include -lnpheap -ltnpheap

clean:
	rm -f hello_bm
