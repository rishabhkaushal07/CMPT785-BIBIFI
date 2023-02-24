// #include <openssl/conf.h>
// #include <openssl/evp.h>
// #include <openssl/err.h>
// #include <openssl/rand.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "headers/json.hpp"
#include<bits/stdc++.h>
using namespace std;
using json = nlohmann::json;

const int BLOCK_SIZE = 16; //bytes
const int KEY_SIZE = 32; //bytes
const int TAG_SIZE = 16; //bytes
const int IV_SIZE = 16; //bytes

void handleErrors(void);
string Randomizer(int ch);
string find_filename(string randomized_filename);
string filename_encryption(string filename);
string filename_decryption(string randomized_filename);
string fetch_randomized_file_path(string filepath);
string fetch_plaintext_file_path(string randomized_filepath);
void encrypt_file(string filePath, string content, unsigned char *key);
string decrypt_file(string filePath, unsigned char *key);


// void handleErrors(void) {
//     ERR_print_errors_fp(stderr);
//     abort();
// }

string Randomizer(int ch){
    // For randomised string generation
    char letters[26] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g',
                          'h', 'i', 'j', 'k', 'l', 'm', 'n',
                          'o', 'p', 'q', 'r', 's', 't', 'u',
                          'v', 'w', 'x', 'y', 'z' };
    string random_string = "";
    for (int i = 0; i<ch; i++)
        random_string = random_string + letters[rand() % 26];
    return random_string;
}

string find_filename(string randomized_filename){
    //opening file using fstream
    std::ifstream metadata_file("metadata.json");
    json metadata_json = json::parse(metadata_file);

    //fetching randomizer-filepath mapping
    string filename = metadata_json.value(randomized_filename, "not found");

    //TODO : error handling
    // if(strcmp(filename,"not found") == 0)
    //     handleErrors();
    return filename;
}

string filename_encryption(string filename){
    srand(time(NULL));
    //Generating the random string for filename
    string randomized_filename = Randomizer(15);

    //Reading the metadata JSON for inserting the randomizer-filename mapping
    std::ifstream metadata_file("metadata.json");
    json metadata_json = json::parse(metadata_file);
    json input_json = json::object();
    input_json[randomized_filename] = filename;

    //Inserting the new mapping into the metadata JSON file
    metadata_json.update(input_json.begin(), input_json.end(), true);
    std::ofstream file("metadata.json");

    //Writing into the metadata JSON file and returning the random string
    file << metadata_json;
    return randomized_filename;
}

string filename_decryption(string randomized_filename){
    string filename;

    //Fetching the filename from the metadata JSON file and returning the filename
    filename = find_filename(randomized_filename);
    return filename;
}

string fetch_randomized_file_path(string filepath){
    char separator = '/';
    int i = 0;
    string randomized_path = "";
    // Temporary string used to split the string.
    string s,temp; 
    while (filepath[i] != '\0') {
        if (filepath[i] != separator) {
            // Append the char to the temp string.
            s += filepath[i]; 
        } else {
            cout << s <<"\n";
            temp = filename_encryption(s);
            randomized_path = randomized_path+ "/" + temp;
            s.clear();
        }
        i++;
    }
    // Output the last stored word.
    temp = filename_encryption(s);
    randomized_path = randomized_path+ "/" + temp;
    return randomized_path;
}

string fetch_plaintext_file_path(string randomized_filepath){
    char separator = '/';
    int i = 0;
    string plaintext_path = "";
    // Temporary string used to split the string.
    string s,temp; 
    while (randomized_filepath[i] != '\0') {
        if (randomized_filepath[i] != separator) {
            // Append the char to the temp string.
            s += randomized_filepath[i]; 
        } else {
            temp = filename_decryption(s);
            plaintext_path = plaintext_path + "/" + temp;
            s.clear();
        }
        i++;
    }
    // Output the last stored word.
    temp = filename_decryption(s);
    plaintext_path = plaintext_path + "/" + s;
    return plaintext_path;
}

// void encrypt_file(string filePath, string content, unsigned char *key) {
//     // generate random iv for each file
//     uint8_t iv[IV_SIZE];
//     RAND_bytes(iv, sizeof(iv));
//     // Buffer for the tag
//     unsigned char tag[TAG_SIZE];

//     // Generate the output file path.
//     // ToDo: invoke the filename_encryption function to get the randomized filepath 
//     // string output_filepath = filename_encryption(filePath);
//     string output_filepath = filePath;

//     // Open the output file for writing.
//     ofstream output_file(output_filepath);
//     if (!output_file) {
//         throw ios_base::failure("Failed to create target file.");
//     }

//     // Initialize the encryption context.
//     EVP_CIPHER_CTX *ctx;
//     if(!(ctx = EVP_CIPHER_CTX_new()))
//         handleErrors();

//     // Initialise the encryption operation
//     if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv))
//         handleErrors();
//     // Set IV length
//     if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL))
//         handleErrors();

//     // Allocate buffers for the plaintext and ciphertext.
//     unsigned char plaintext[BLOCK_SIZE];
//     unsigned char ciphertext[BLOCK_SIZE + EVP_MAX_BLOCK_LENGTH];
//     int len = 0;
//     int ciphertext_len = 0;

//     // allocate space for iv and tag at the beginning of the output file
//     output_file.write("0", IV_SIZE);
//     output_file.write("0", TAG_SIZE);

//     // Read data from the input content string and encrypt it in chunks.
//     size_t pos = 0;
//     size_t length = content.length();
//     while (pos < length) {
//         int ptlen = length - pos;
//         if(ptlen >= BLOCK_SIZE) {
//             content.substr(pos, BLOCK_SIZE).copy((char*)plaintext, BLOCK_SIZE);
//             ptlen = BLOCK_SIZE;
//         }
//         else {
//             content.substr(pos, ptlen).copy((char*)plaintext, ptlen);
//         }
//         pos += ptlen;
        
//         if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, ptlen)) {
//             handleErrors();
//         }
//         ciphertext_len = len;
//         output_file.write((char*)ciphertext, ciphertext_len);
//     }

//     // Finalize the encryption.
//     if (1 != EVP_EncryptFinal_ex(ctx, ciphertext, &len)) {
//         handleErrors();
//     }
//     ciphertext_len = len;
//     output_file.write((char*)ciphertext, ciphertext_len);

//     // Get the tag
//     if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_SIZE, tag))
//         handleErrors();

//     // write iv and tag to the output file
//     output_file.seekp(0);
//     output_file.write((char*)iv, IV_SIZE);
//     output_file.write((char*)tag, TAG_SIZE);

//     // Clean up the context and close the files.
//     EVP_CIPHER_CTX_free(ctx);
//     output_file.close();
// }

// string decrypt_file(string filePath, unsigned char *key) {
//     string ptoutput = "";
//     // Open the input file for reading.
//     // ToDo: Invoke the filename_decryption function to get the mapping of the plaintext filename
//     // ifstream input_file(filename_decryption(filePath));
//     ifstream input_file(filePath);
//     if (!input_file) {
//         throw ios_base::failure("Failed to open file: " + filePath);
//     }

//     // Read tag and IV from the file.
//     uint8_t iv[IV_SIZE];
//     uint8_t tag[TAG_SIZE];
//     input_file.read((char*)iv, IV_SIZE);
//     input_file.read((char*)tag, TAG_SIZE);

//     // Initialize the decryption context.
//     EVP_CIPHER_CTX *ctx;
//     if (!(ctx = EVP_CIPHER_CTX_new())) {
//         handleErrors();
//     }

//     // Initialise the decryption operation
//     if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv)) {
//         handleErrors();
//     }

//     // Set IV length
//     if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_SIZE, NULL)) {
//         handleErrors();
//     }

//     // Allocate buffers for the plaintext and ciphertext.
//     unsigned char ciphertext[BLOCK_SIZE + EVP_MAX_BLOCK_LENGTH];
//     unsigned char decryptedtext[BLOCK_SIZE];
//     int len = 0;
//     // Read data from the input file and decrypt it in chunks.
//     while (input_file) {
//         input_file.read((char*)ciphertext, BLOCK_SIZE);
//         if (1 != EVP_DecryptUpdate(ctx, decryptedtext, &len, ciphertext, input_file.gcount())) {
//             handleErrors();
//         }
//         string str((char*)decryptedtext, len);
//         ptoutput.append(str);
//     }

//     // Set expected tag
//     if (!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_SIZE, (void *)tag)) {
//         handleErrors();
//     }

//     // Finalize the decryption.
//     // This step verifies if tag is fine and errors out in case of changes to encrypted files.
//     if (1 != EVP_DecryptFinal_ex(ctx, decryptedtext, &len)) {
//         handleErrors();
//     }
//     string str((char*)decryptedtext, len);
//     ptoutput.append(str);

//     // Clean up the context and close the files.
//     EVP_CIPHER_CTX_free(ctx);
//     input_file.close();
//     return ptoutput;
// }

int main()
{
    string filepath ="dir1/dir2/filename.txt";
    string output;
    output = fetch_randomized_file_path(filepath);
    cout<<output;
    cout << fetch_plaintext_file_path(output);
}
