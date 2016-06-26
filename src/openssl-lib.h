int encrypt_wrapper(const BYTE *data, int size, const BYTE *password, BYTE* ans, encryption_method method, block_method block);
int decrypt_wrapper(const BYTE *data, int size, const BYTE *password, BYTE* ans, encryption_method method, block_method block);
