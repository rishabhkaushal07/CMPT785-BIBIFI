#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
using namespace std;

const int BLOCK_SIZE = 16; //bytes
const int KEY_SIZE = 32; //bytes
const int TAG_SIZE = 16; //bytes
const int IV_SIZE = 16; //bytes

void handleErrors(void);
void encrypt_file(string filePath);
void decrypt_file(string filePath);

streampos get_file_size(fstream file){
    file.seekg(0, file.end);
    streampos file_size = file.tellg();
    file.seekg(0, file.beg);
    return file_size;
}

void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

void encrypt_file(string filePath) {
    uint8_t key[KEY_SIZE];
    uint8_t iv[IV_SIZE];
    RAND_bytes(key, sizeof(key));
    RAND_bytes(iv, sizeof(iv));

    // read the file to be encrypted
    fstream file (filePath);
    if (!file.is_open()) {
        throw ios_base::failure("Failed to open file: " + filePath);
    }
    file.seekg(0, file.end);
    streampos file_size = file.tellg();
    file.seekg(0, file.beg);   
    // ToDo: fix get_file_size call
    // streampos file_size = get_file_size(file);
    unsigned char *plaintext = new unsigned char[file_size];
    file.read((char *)plaintext, file_size);
    file.close();
    
    // Calculate the buffer size needed for ciphertext and allocate the space
    int ciphertextBufferSize = ceil(file_size / (double) BLOCK_SIZE) * BLOCK_SIZE;
    unsigned char ciphertext[ciphertextBufferSize];
    // Buffer for the tag
    unsigned char tag[TAG_SIZE];

    // Create and initialise the context
    EVP_CIPHER_CTX *ctx;
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();
    int len;
    int ciphertext_len;

    // Initialise the encryption operation
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
        handleErrors();
    // Set IV length
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL))
        handleErrors();
    // Initialise key and IV
    if(1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv))
        handleErrors();

    // Encrypt the file contents 
    // ToDo: encrypt block size chunks recursively
    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, strlen ((char *)plaintext)))
        handleErrors();
    ciphertext_len = len;

    // Finalise the encryption
    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        handleErrors();
    ciphertext_len += len;

    // Get the tag
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag))
        handleErrors();

    // To Do
    // write the ciphertext to file on FS
    // save key and iv
    // save the tag for verifying before decrypting

    // Clean up
    EVP_CIPHER_CTX_free(ctx);
}

void decrypt_file(string filePath) {
    // ToDo: get key and iv from the persistent store
    uint8_t key[KEY_SIZE];
    uint8_t iv[IV_SIZE];

    unsigned char tag[TAG_SIZE];
    
    // read the file to be decrypted
    fstream file (filePath);
    if (!file.is_open()) {
        throw ios_base::failure("Failed to open file: " + filePath);
    }
    file.seekg(0, file.end);
    streampos file_size = file.tellg();
    file.seekg(0, file.beg);   
    // ToDo: fix get_file_size call
    // streampos file_size = get_file_size(file);
    unsigned char *ciphertext = new unsigned char[file_size];
    file.read((char *)ciphertext, file_size);
    file.close();    
    
    // Buffer for the decrypted text
    unsigned char decryptedtext[(int) file_size];
    int decryptedtext_len, ciphertext_len;

    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;
    int ret;

    // Create and initialise the context
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    // Initialise the decryption operation
    if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
        handleErrors();

    // Set IV length
    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL))
        handleErrors();

    // Initialise key and IV
    if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv))
        handleErrors();

    // Decrypt the file contents 
    // ToDo: decrypt block size chunks recursively
    if(!EVP_DecryptUpdate(ctx, decryptedtext, &len, ciphertext, ciphertext_len))
        handleErrors();
    plaintext_len = len;

    // Set expected tag value.
    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, tag))
        handleErrors();

    // Finalise the decryption
    ret = EVP_DecryptFinal_ex(ctx, decryptedtext + len, &len);

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    // Add a NULL terminator. We are expecting printable text
    decryptedtext[decryptedtext_len] = '\0';
}
