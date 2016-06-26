#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/obj_mac.h>
#include <openssl/aes.h>
#include <openssl/conf.h>
#include <openssl/err.h>
#include <openssl/md5.h>



int getBlockSize(block_method block){
	switch(block){
		case AES128: return 16;
		case AES192: return 16;
		case AES256: return 16;
		case DES: return 16;
	}
	return -1;
}

encrypt_function getFunction(encryption_method method, block_method block){
	switch(block){
		case ECB:
			switch(method){
				case AES128: return EVP_aes_128_ecb;
				case AES192: return EVP_aes_192_ecb;
				case AES256: return EVP_aes_256_ecb;
				case DES: return EVP_des_ecb;
			}
		case CFB:
			switch(method){
				case AES128: return EVP_aes_128_cfb8;
				case AES192: return EVP_aes_192_cfb8;
				case AES256: return EVP_aes_256_cfb8;
				case DES: return EVP_des_cfb8;
			}
		case OFB:
			switch(method){
				case AES128: return EVP_aes_128_ofb;
				case AES192: return EVP_aes_192_ofb;
				case AES256: return EVP_aes_256_ofb;
				case DES: return EVP_des_ofb;
			}
		case CBC:
			switch(method){
				case AES128: return EVP_aes_128_cbc;
				case AES192: return EVP_aes_192_cbc;
				case AES256: return EVP_aes_256_cbc;
				case DES: return EVP_des_cbc;
			}
	}
	return NULL;
}

int encrypt(const BYTE* data, long size, const BYTE *password, int pass_size, BYTE* ans, encrypt_function function) {
	EVP_CIPHER_CTX *ctx =EVP_CIPHER_CTX_new();
	int outl, templ;
	BYTE *out=ans;
	BYTE key[EVP_CIPHER_key_length(function())];
    BYTE iv[EVP_CIPHER_iv_length(function())];
	/* Getting keys and iv */ 
	// Salt is setting in NULL
	EVP_BytesToKey(function(), EVP_md5(), NULL, password, strlen((char*)password),1, key, iv);
    /* Initialize context */
    EVP_CIPHER_CTX_init(ctx);
	EVP_EncryptInit_ex(ctx, function(), NULL, key, iv); 
	EVP_EncryptUpdate(ctx, out, &outl, data, size); 
	EVP_EncryptFinal_ex(ctx, out + outl, &templ);
	outl +=templ;
	//memcpy(ans, out, outl);
	/* Clean context struct */ 
	EVP_CIPHER_CTX_cleanup(ctx);
	return outl;
}

int decrypt(const BYTE* data, long size, const BYTE *password, int pass_size, BYTE* ans, encrypt_function function) {
    /*EVP_CIPHER_CTX ctx;
	BYTE out[size]; 
	int outl, templ;
	unsigned int keyLen, ivLen;
	keyLen = EVP_CIPHER_key_length(function());
	ivLen = EVP_CIPHER_iv_length(function());
	BYTE key[keyLen];
	BYTE iv[ivLen];
	EVP_BytesToKey(function(), EVP_md5(), NULL, password, pass_size,1, key, iv);
	EVP_CIPHER_CTX_init(&ctx);
	EVP_DecryptInit_ex(&ctx, function(), NULL, key, iv);
	EVP_DecryptUpdate(&ctx, out, &outl, data, size);
	EVP_DecryptFinal_ex(&ctx, out + outl, &templ); 
	outl +=templ;
	memcpy(ans, out, outl);
	EVP_CIPHER_CTX_cleanup(&ctx);*/
	EVP_CIPHER_CTX ctx;
	BYTE *out=ans; 
	int outl, templ;
    BYTE key[EVP_CIPHER_key_length(function())];
    BYTE iv[EVP_CIPHER_iv_length(function())];

	// Getting keys and iv 
	// Salt is setting in NULL
    EVP_BytesToKey(function(), EVP_md5(), NULL, password, pass_size,1, key, iv);

	// Initialize context 
	EVP_CIPHER_CTX_init(&ctx);
	EVP_DecryptInit_ex(&ctx, function(), NULL, key, iv); 
	EVP_DecryptUpdate(&ctx, out, &outl, data, size); 
	EVP_DecryptFinal_ex(&ctx, out + outl, &templ);
	outl +=templ;
	//memcpy(ans, out, outl);
	//ans[outl]=0; //Maybe?
	// Clean context struct 
	EVP_CIPHER_CTX_cleanup(&ctx);
	return outl;
}

int encrypt_wrapper(const BYTE *data, long size, const BYTE *password, BYTE* ans, encryption_method method, block_method block){
	return encrypt(data, size, password, strlen((char*)password),ans, getFunction(method, block));
}

int decrypt_wrapper(const BYTE *data, long size, const BYTE *password, BYTE* ans, encryption_method method, block_method block){
	return decrypt(data, size, password, strlen((char*)password),ans, getFunction(method, block));
}

/*int main(int argc, char**argv){
	char msg[1000];
	BYTE ans[1000];
	BYTE ans2[1000];
	scanf("%s", msg);
	FILE* f = fopen(msg, "r");
	FILE* out = fopen("encripted.txt", "w");
	fread(ans+4, 11, 1, f);
	ans[0]=0;
	ans[1]=0;
	ans[2]=0;
	ans[3]=11;
	ans[15]='.';
	ans[16]='t';
	ans[17]='x';
	ans[18]='t';
	int size= encrypt_wrapper((BYTE*)ans, 19, (BYTE*)"nada", (BYTE*)ans2, AES192, CBC);
	fwrite(ans2, size, 1, out);
	fclose(out);
	return 0;
}*/

