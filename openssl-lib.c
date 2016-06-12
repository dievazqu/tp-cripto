#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/aes.h>


int getBlockSize(block_method block){
	switch(block){
		case AES128: return 128;
		case AES192: return 192;
		case AES256: return 256;
		case DES: return 64;
	}
	return -1;
}

encrypt_function getFunction(block_method block, encryption_method method){
	switch(block){
		case ECB:
			switch(method){
				case AES128: return EVP_aes_128_ecb;
				case AES192: return EVP_aes_192_ecb;
				case AES256: return EVP_aes_256_ecb;
				case DES: return EVP_des_ede3_ecb;
			}
		case CFB:
			switch(method){
				case AES128: return EVP_aes_128_cfb8;
				case AES192: return EVP_aes_192_cfb8;
				case AES256: return EVP_aes_256_cfb8;
				case DES: return EVP_des_ede3_cfb8;
			}
		case OFB:
			switch(method){
				case AES128: return EVP_aes_128_ofb;
				case AES192: return EVP_aes_192_ofb;
				case AES256: return EVP_aes_256_ofb;
				case DES: return EVP_des_ede3_ofb;
			}
		case CBC:
			switch(method){
				case AES128: return EVP_aes_128_cbc;
				case AES192: return EVP_aes_192_cbc;
				case AES256: return EVP_aes_256_cbc;
				case DES: return EVP_des_ede3_cbc;
			}
	}
	return NULL;
}

int encrypt(const BYTE* data, int size, const BYTE *password, int pass_size, BYTE* ans, encrypt_function function) {
	EVP_CIPHER_CTX ctx;
	int outl, templ;
	BYTE out[size];
	BYTE key[pass_size];
	BYTE iv[pass_size];
	
	/* Getting keys and iv */ 
	// Salt is setting in NULL
	EVP_BytesToKey(function(), EVP_md5(), NULL, password, strlen((char*)password),1, key, iv);

    /* Initialize context */
    EVP_CIPHER_CTX_init(&ctx);
	EVP_EncryptInit_ex(&ctx, function(), NULL, key, iv); 
	EVP_EncryptUpdate(&ctx, out, &outl, data, size); 
	EVP_EncryptFinal_ex(&ctx, out + outl, &templ);
	outl +=templ;
	memcpy(ans, out, outl);
	/* Clean context struct */ 
	EVP_CIPHER_CTX_cleanup(&ctx);
	return outl;
}

int decrypt(const BYTE* data, int size, const BYTE *password, int pass_size, BYTE* ans, encrypt_function function) {
	EVP_CIPHER_CTX ctx;
	BYTE out[size]; 
	int outl, templ;
    BYTE key[pass_size];
    BYTE iv[pass_size];

	/* Getting keys and iv */ 
	// Salt is setting in NULL
    EVP_BytesToKey(function(), EVP_md5(), NULL, password, strlen((char*)password),1, key, iv);

	/* Initialize context */
	EVP_CIPHER_CTX_init(&ctx);
	EVP_DecryptInit_ex(&ctx, function(), NULL, key, iv); 
	EVP_DecryptUpdate(&ctx, out, &outl, data, size); 
	EVP_DecryptFinal_ex(&ctx, out + outl, &templ);
	outl +=templ;
	memcpy(ans, out, outl);
	/* Clean context struct */ 
	EVP_CIPHER_CTX_cleanup(&ctx);
	return outl;
}

int encrypt_wrapper(const BYTE *data, int size, const BYTE *password, BYTE* ans, encryption_method method, block_method block){
	return encrypt(data, size, password, getBlockSize(block),ans, getFunction(method, block));
}

int decrypt_wrapper(const BYTE *data, int size, const BYTE *password, BYTE* ans, encryption_method method, block_method block){
	return decrypt(data, size, password, getBlockSize(block),ans, getFunction(method, block));
}
/*
int main(int argc, char**argv){
	char msg[100];
	char ans[100];
	char ans2[100];
	scanf("%s", msg);
	encrypt_wrapper((BYTE*)msg, strlen(msg), (BYTE*)"nada", ans, AES128, CBC);
	decrypt_wrapper((BYTE*)ans, strlen(ans), (BYTE*)"nada", ans2, AES128, CBC);
	printf("%s %d\n", ans, strlen(ans));
	printf("%s %d\n", ans2, strlen(ans2));
	int i=0;
	while(ans2[i]) {printf("%02x ", (unsigned char)ans2[i]); i++;}
	return 0;
}
*/
