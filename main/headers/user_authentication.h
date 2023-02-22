// user authentication

#ifndef CMPT785_BIBIFI_USER_AUTHENTICATION_H
#define CMPT785_BIBIFI_USER_AUTHENTICATION_H

#include "encryption.h"
#include <string>
#include <random>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

using namespace std;
void add_user(const string& username, const filesystem::path& public_key_dir, const filesystem::path& private_key_dir, bool admin = false) {
    // Generate RSA key pair
    RSA* rsa = RSA_new();
    BIGNUM* bn = BN_new();
    BN_set_word(bn, RSA_F4);
    RSA_generate_key_ex(rsa, 2048, bn, NULL);

    // Create EVP_PKEY object
    EVP_PKEY* pkey = EVP_PKEY_new();
    if (!pkey) {
        cerr << "Failed to create EVP_PKEY object" << endl;
        RSA_free(rsa);
        BN_free(bn);
        return;
    }
    EVP_PKEY_set1_RSA(pkey, rsa);

    // Get public key
    unsigned char* pub_key_der;
    int pub_key_der_len = i2d_PublicKey(pkey, &pub_key_der);

    // Write public key to file
    string pub_key_filename = username + ".pub";
    if (admin) {
        pub_key_filename = "admin.pub";
    }
    filesystem::path pub_key_file = public_key_dir / pub_key_filename;
    ofstream pub_key_stream(pub_key_file, ios::binary);
    pub_key_stream.write((char*)pub_key_der, pub_key_der_len);
    pub_key_stream.close();

    // Get private key
    unsigned char* pri_key_der;
    int pri_key_der_len = i2d_PrivateKey(pkey, &pri_key_der);

    // Write private key to file
    string pri_key_filename = username + ".pri";
    if (admin) {
        pri_key_filename = "admin.pri";
    }
    filesystem::path pri_key_file = private_key_dir / pri_key_filename;
    ofstream pri_key_stream(pri_key_file, ios::binary);
    pri_key_stream.write((char*)pri_key_der, pri_key_der_len);
    pri_key_stream.close();

    // Clean up
    RSA_free(rsa);
    BN_free(bn);
    EVP_PKEY_free(pkey);

    cout << "User " << username << " added successfully." << endl;
}

string get_type_of_user(const std::string &keyfile_name) {

  // First check if the keyfile is a valid file for or not
  if (is_valid_keyfile(keyfile_name)) {

    // After authenticating that the keyfile is valid and appropriate
    // Decrypt the filesystem and,
    decrypt_filesystem();

    // Return the type of the user based on the keyfile
    string username;
    if (keyfile_name == "admin_keyfile_name") {
      username = "admin";
    } else {
      username = "user";
    }

    cout << "Logged in as " << username << endl;
    return username;
  }

  // Since the user wasn't authenticated, the login was failed and the program was exited.
  cout << "Invalid keyfile" << endl;
  // Before exiting encrypt the filesystem again
  encrypt_filesystem();
  exit(EXIT_FAILURE);
}

// TODO: Implement authentication, currently it just checks that keyfile should not be empty

bool is_valid_keyfile(const string &keyfile_name) {
    EVP_PKEY* pkey = nullptr;
    FILE* fp = nullptr;
    bool result = false;

    // Open the keyfile for reading
    fp = fopen(keyfile_name.c_str(), "rb");
    if (!fp) {
        std::cerr << "Error opening keyfile: " << keyfile_name << std::endl;
        goto cleanup;
    }

    // Read the public key from the keyfile
    pkey = PEM_read_PUBKEY(fp, nullptr, nullptr, nullptr);
    if (!pkey) {
        std::cerr << "Error reading public key from keyfile: " << keyfile_name << std::endl;
        goto cleanup;
    }

    // If we got this far, the keyfile is valid
    result = true;

cleanup:
    // Clean up resources
    if (fp) {
        fclose(fp);
    }
    if (pkey) {
        EVP_PKEY_free(pkey);
    }
    // before exiting encrypt the filesystem again
    encrypt_filesystem();
    return result;
}

#endif // CMPT785_BIBIFI_USER_AUTHENTICATION_H
