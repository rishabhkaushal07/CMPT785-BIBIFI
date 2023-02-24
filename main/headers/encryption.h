#ifndef CMPT785_BIBIFI_ENCRYPTION_H
#define CMPT785_BIBIFI_ENCRYPTION_H

const int BLOCK_SIZE = 16; //bytes
const int KEY_SIZE = 32; //bytes
const int TAG_SIZE = 16; //bytes
const int IV_SIZE = 16; //bytes

void handleErrors(void);
void encrypt_file(string filePath, string content, unsigned char *key);
string decrypt_file(string filePath, unsigned char *key);

#endif // CMPT785_BIBIFI_ENCRYPTION_H
