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
void encrypt_file(string filePath, string content, unsigned char *key);
string decrypt_file(string filePath, unsigned char *key);

void handleErrors(void) {
    ERR_print_errors_fp(stderr);
    abort();
}

void ecryption(user, key){
    if(user == 'admin'){
        encrypt_user_files('/', key)
    }
    else if (user != NULL){
        encrypt_user_files(user+'/', key)
    }
    else{
        handleErrors();
    }
}

void encrypt_user_files(dir_path, key){
    if (!exists(dir_path)) 
        handleErrors();
    else{
        len = strlen(name);
        dirp = opendir(dir_path);
        while ((dp = readdir(dirp)) != NULL)
            encrypt_file(dp->d_name, key)
        (void)closedir(dirp);
    }
}

void encrypt_file(string filePath, string content, unsigned char *key) {
    // generate random iv for each file
    uint8_t iv[IV_SIZE];
    RAND_bytes(iv, sizeof(iv));
    // Buffer for the tag
    unsigned char tag[TAG_SIZE];

    // Generate the output file path.
    string output_filepath = filename_encryption(filePath);

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

    // allocate space for iv and tag at the beginning of the output file
    output_file.write("0", IV_SIZE);
    output_file.write("0", TAG_SIZE);

    // Read data from the input content string and encrypt it in chunks.
    size_t pos = 0;
    size_t length = content.length();
    while (pos < length) {
        int ptlen = length - pos;
        if(ptlen >= BLOCK_SIZE) {
            content.substr(pos, BLOCK_SIZE).copy((char*)plaintext, BLOCK_SIZE);
            ptlen = BLOCK_SIZE;
        }
        else {
            content.substr(pos, ptlen).copy((char*)plaintext, ptlen);
        }
        pos += ptlen;
        
        if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, ptlen)) {
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
    output_file.close();
}

string decrypt_file(string filePath, unsigned char *key) {
    string ptoutput = "";
    // Open the input file for reading.
    ifstream input_file(filename_decryption(filePath));
    if (!input_file) {
        throw ios_base::failure("Failed to open file: " + filePath);
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
    // Read data from the input file and decrypt it in chunks.
    while (input_file) {
        input_file.read((char*)ciphertext, BLOCK_SIZE);
        if (1 != EVP_DecryptUpdate(ctx, decryptedtext, &len, ciphertext, input_file.gcount())) {
            handleErrors();
        }
        string str((char*)decryptedtext, len);
        ptoutput.append(str);
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
    string str((char*)decryptedtext, len);
    ptoutput.append(str);

    // Clean up the context and close the files.
    EVP_CIPHER_CTX_free(ctx);
    input_file.close();
    return ptoutput;
}