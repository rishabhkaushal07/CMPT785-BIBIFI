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
void add_user(const std::string& username, const std::filesystem::path& public_key_dir, const std::filesystem::path& key_file_dir, bool admin = false) {
    if (admin) {
        // Generate admin SSH key pair
        std::string admin_username = "admin";
        std::filesystem::path admin_pub_key_file = public_key_dir / (admin_username + ".pub");
        std::filesystem::path admin_pri_key_file = key_file_dir / (admin_username + ".pri");

        std::string admin_ssh_keygen_cmd = "ssh-keygen -t rsa -b 2048 -f " + admin_pri_key_file.string() + " -N '' -q";
        system(admin_ssh_keygen_cmd.c_str());

        // Move public key to public key directory
        std::filesystem::path admin_pub_key_temp_file = admin_pri_key_file.parent_path() / (admin_username + ".pri.pub");
        std::filesystem::rename(admin_pub_key_temp_file, admin_pub_key_file);
        for (const auto& entry : std::filesystem::directory_iterator(key_file_dir)) {
            std::cout << entry.path() << '\n';
        }
        for (const auto& entry : std::filesystem::directory_iterator(public_key_dir)) {
            std::cout << entry.path() << '\n';
        }
        // Move private key to keyfile directory
        std::filesystem::rename(admin_pri_key_file, key_file_dir / (admin_username + "_keyfile"));

        std::cout << "Admin key pair generated successfully." << std::endl;
        return;
    }
    std::filesystem::path pub_key_file = public_key_dir / (username + ".pub");
    std::filesystem::path pri_key_file = key_file_dir / (username + ".pri");
    // Generate SSH key pair
    std::string ssh_keygen_cmd = "ssh-keygen -t rsa -b 2048 -f " + pri_key_file.string() + " -N '' -q";
    system(ssh_keygen_cmd.c_str());

    // Move public key to public key directory
    std::filesystem::path pub_key_temp_file = pri_key_file.parent_path() / (username + ".pri.pub");
    std::filesystem::rename(pub_key_temp_file, pub_key_file);

    // Move private key to keyfile directory
    std::filesystem::rename(pri_key_file, key_file_dir / (username + "_keyfile"));
    for (const auto& entry : std::filesystem::directory_iterator(key_file_dir)) {
        std::cout << entry.path() << '\n';
    }
    for (const auto& entry : std::filesystem::directory_iterator(public_key_dir)) {
        std::cout << entry.path() << '\n';
    }

    std::cout << "User " << username << " added successfully." << std::endl;
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
