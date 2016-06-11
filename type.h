#include <openssl/evp.h>

typedef unsigned char BYTE;
typedef const EVP_CIPHER*(*encrypt_function)();
typedef enum { 
	ECB, 
	CFB, 
	OFB,
	CBC 
} block_method;
typedef enum {
	AES128,
	AES192,
	AES256,
	DES
} encryption_method;
