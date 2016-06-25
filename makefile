default:	
	rm -f ./stegowav
	gcc -g -Wall -I/usr/include/openssl -c stegowav.c -o stegowav.o
	gcc -g -Wall -I/usr/include/openssl -c openssl-lib.c -o openssl-lib.o
	gcc stegowav.o openssl-lib.o -lcrypto -Wall -lm -o stegowav
