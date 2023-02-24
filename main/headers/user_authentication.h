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
#include <regex>

using namespace std;
void add_user(const std::string& username, bool admin=false) {
    // Check if admin creation, when filesystem first start admin=true, after that admin is not allowed as username
    if(!admin){
        if(username == "admin"){
            std::cout << "Error: Invalid Username." << std::endl;
        return;
        }
    }
    // Check if the username is super long
    if (username.length() > 50) {
        std::cout << "Error: Username is too long." << std::endl;
        return;
    }
    
    // Check that the username only contains letters, numbers, and underscores
    std::regex username_regex("^[a-zA-Z0-9_]*$");
    if (!std::regex_match(username, username_regex)) {
        std::cout << "Error: Username contains invalid characters." << std::endl;
        return;
    }
    // Check if the user already exist
    if (std::filesystem::exists("public_keys/" + (username + ".pub")) || std::filesystem::exists("private_keys/" + (username + "_keyfile"))) {
        std::cout << "User " << username << " already exists." << std::endl;
        return;
    }
    
    std::filesystem::path pri_key_file = "private_keys/" + username;
    // Generate SSH key pair
    std::string ssh_keygen_cmd = "ssh-keygen -t rsa -b 2048 -f " + pri_key_file.string() + " -N '' -q";
    system(ssh_keygen_cmd.c_str());
    // Move public key to public key directory
    std::filesystem::path temp_file = "private_keys/" + (username + ".pub");
    std::filesystem::rename(temp_file, "public_keys/" + (username + ".pub"));
    
    // Rename private key
    std::filesystem::rename("private_keys/" + username, ("private_keys/" + username +"_keyfile"));

    std::cout << "User " << username << " added successfully." << std::endl;
}

string get_type_of_user(const std::string &keyfile_name) {

  // First check if the keyfile is a valid file for or not
  //if (is_valid_keyfile(keyfile_name)) {

    // After authenticating that the keyfile is valid and appropriate
    // Decrypt the filesystem and,
    decrypt_filesystem();

    // Return the type of the user based on the keyfile
    string username = keyfile_name;
    if (username == "admin_keyfile") {
      username = "admin";
    } else {
      username.erase(username.find_first_of("_ "));
    }
    cout << "Logged in as " << username << endl;
    return username;
  }
  /*
  // Since the user wasn't authenticated, the login was failed and the program was exited.
  cout << "Invalid keyfile" << endl;
  // Before exiting encrypt the filesystem again
  encrypt_filesystem();
  exit(EXIT_FAILURE);
  */
}

// TODO: Implement authentication, currently it just checks that keyfile should not be empty
/*
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
    */
    // before exiting encrypt the filesystem again
    encrypt_filesystem();
    return result;
}

#endif // CMPT785_BIBIFI_USER_AUTHENTICATION_H
