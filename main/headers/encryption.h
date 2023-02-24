#ifndef CMPT785_BIBIFI_ENCRYPTION_H
#define CMPT785_BIBIFI_ENCRYPTION_H

using namespace std;

void handleErrors(void);
void encrypt_file(string filePath, string content, unsigned char *key);
string decrypt_file(string filePath, unsigned char *key);

#endif // CMPT785_BIBIFI_ENCRYPTION_H
