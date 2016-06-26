default:	
	rm -f ./stegowav
	gcc -g -Wall -I/usr/include/openssl -c src/stegowav.c -o obj/stegowav.o
	gcc -g -Wall -I/usr/include/openssl -c src/openssl-lib.c -o obj/openssl-lib.o
	gcc obj/stegowav.o obj/openssl-lib.o -lcrypto -Wall -lm -o stegowav
