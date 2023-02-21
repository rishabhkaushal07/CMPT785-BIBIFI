#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <string.h>
#include <iostream>
#include <fstream>
using namespace std;

const int BLOCK_SIZE = 16; //bytes
const int KEY_SIZE = 32; //bytes
const int TAG_SIZE = 16; //bytes
const int IV_SIZE = 16; //bytes

void handleErrors(void);
void encrypt_file(string filePath, unsigned char *key);
void decrypt_file(string filePath, unsigned char *key);

void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}

void encrypt_file(string filePath, unsigned char *key) {
    // generate random iv for each file
    uint8_t iv[IV_SIZE];
    RAND_bytes(iv, sizeof(iv));
    // Buffer for the tag
    unsigned char tag[TAG_SIZE];

    // Open the input file for reading.
    ifstream input_file(filePath);
    if (!input_file) {
        throw ios_base::failure("Failed to open file: " + filePath);
    }

    // Generate the output file path.
    string output_filepath = filePath + ".enc";

    // Open the output file for writing.
    ofstream output_file(output_filepath);
    if (!output_file) {
        throw ios_base::failure("Failed to create target file.");
    }

    // Initialize the encryption context.
    EVP_CIPHER_CTX *ctx;
    if(!(ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    // Initialise the encryption operation
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv))
        handleErrors();
    // Set IV length
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL))
        handleErrors();

    // Allocate buffers for the plaintext and ciphertext.
    unsigned char plaintext[BLOCK_SIZE];
    unsigned char ciphertext[BLOCK_SIZE + EVP_MAX_BLOCK_LENGTH];
    int len = 0;
    int ciphertext_len = 0;

    // leave space for iv and tag at the beginning of the output file
    output_file.write("0", IV_SIZE);
    output_file.write("0", TAG_SIZE);

    // Read data from the input file and encrypt it in chunks.
    while (input_file) {
        input_file.read((char*)plaintext, BLOCK_SIZE);
        if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, input_file.gcount())) {
            handleErrors();
        }
        ciphertext_len = len;
        output_file.write((char*)ciphertext, ciphertext_len);
    }

    // Finalize the encryption.
    if (1 != EVP_EncryptFinal_ex(ctx, ciphertext, &len)) {
        handleErrors();
    }
    ciphertext_len = len;
    output_file.write((char*)ciphertext, ciphertext_len);

    // Get the tag
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag))
        handleErrors();

    // write iv and tag to the output file
    output_file.seekp(0);
    output_file.write((char*)iv, IV_SIZE);
    output_file.write((char*)tag, TAG_SIZE);

    // Clean up the context and close the files.
    EVP_CIPHER_CTX_free(ctx);
    input_file.close();
    output_file.close();
}

void decrypt_file(string filePath, unsigned char *key) {
    // Generate the output file path.
    string output_filepath = filePath + ".dec";

    // Open the input file for reading.
    ifstream input_file(filePath);
    if (!input_file) {
        throw ios_base::failure("Failed to open file: " + filePath);
    }

    // Open the output file for writing.
    ofstream output_file(output_filepath);
    if (!output_file) {
        throw ios_base::failure("Failed to create target file.");
    }

    // Read tag and IV from the file.
    uint8_t iv[IV_SIZE];
    uint8_t tag[TAG_SIZE];
    input_file.read((char*)iv, IV_SIZE);
    input_file.read((char*)tag, TAG_SIZE);

    // Initialize the decryption context.
    EVP_CIPHER_CTX *ctx;
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        handleErrors();
    }

    // Initialise the decryption operation
    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv)) {
        handleErrors();
    }

    // Set IV length
    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL)) {
        handleErrors();
    }

    // Allocate buffers for the plaintext and ciphertext.
    unsigned char ciphertext[BLOCK_SIZE + EVP_MAX_BLOCK_LENGTH];
    unsigned char decryptedtext[BLOCK_SIZE];
    int len = 0;
    int decryptedtext_len = 32;
    // Read data from the input file and decrypt it in chunks.
    while (input_file) {
        input_file.read((char*)ciphertext, BLOCK_SIZE);
        if (1 != EVP_DecryptUpdate(ctx, decryptedtext, &len, ciphertext, input_file.gcount())) {
            handleErrors();
        }
        decryptedtext_len = len;
        output_file.write((char*)decryptedtext, decryptedtext_len);
    }

    // Set expected tag
    if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, (void *)tag)) {
        handleErrors();
    }

    // Finalize the decryption.
    // This step verifies if tag is fine and errors out in case of changes to encrypted files.
    if (1 != EVP_DecryptFinal_ex(ctx, decryptedtext, &len)) {
        handleErrors();
    }
    decryptedtext_len = len;
    output_file.write((char*)decryptedtext, decryptedtext_len);

    // Clean up the context and close the files.
    EVP_CIPHER_CTX_free(ctx);
    input_file.close();
    output_file.close();
}