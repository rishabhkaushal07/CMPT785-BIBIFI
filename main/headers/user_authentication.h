// user authentication

#ifndef CMPT785_BIBIFI_USER_AUTHENTICATION_H
#define CMPT785_BIBIFI_USER_AUTHENTICATION_H

#include "helper_functions.h"
#include "encryption.h"
#include <string>
#include <random>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>
#include "helper_functions.h"

using namespace std;
void add_user(const string& username, string path, bool admin=false)
{
    path = path + "/";
    // Check if admin creation, when filesystem first start admin=true, after that admin is not allowed as username
    if(!admin) {
        if(username == "admin") {
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
    if (std::filesystem::exists(path + "public_keys/" + (username + ".pub")) || std::filesystem::exists(path + "private_keys/" + (username + "_keyfile"))) {
        std::cout << "User " << username << " already exists." << std::endl;
        return;
    }

    std::filesystem::path pri_key_file = path + "private_keys/" + username;
    // Generate SSH key pair
    std::string ssh_keygen_cmd = "ssh-keygen -t rsa -b 2048 -f " + pri_key_file.string() + " -N '' -q";
    system(ssh_keygen_cmd.c_str());
    // Move public key to public key directory
    std::filesystem::path temp_file = path + "private_keys/" + (username + ".pub");
    std::filesystem::rename(temp_file, path + "public_keys/" + (username + ".pub"));

    // Rename private key
    std::filesystem::rename(path + "private_keys/" + username, (path + "private_keys/" + username + "_keyfile"));

    std::cout << "User " << username << " added successfully." << std::endl;
    add_enc_key_to_metadata(username, path);
    create_init_fs_for_user(username, path);
}

bool is_valid_keyfile(const string &username)
{
    string name = username;
    is_valid_filename(username);
    
    // Paths for private key and public key
    std::filesystem::path private_key_path = "private_keys/" + name + "_keyfile";
    std::filesystem::path public_key_path = "public_keys/" + name + ".pub";
    
    // Extract the public key from the private key
    std::string command = "ssh-keygen -y -f " + private_key_path.string();
    uint8_t buffer[128];
    std::string expected_public_key;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        std::cerr << "Failed to run command: " << command << std::endl;
        return false;
    }
    while (fgets((char*)buffer, 128, pipe.get()) != nullptr) {
        expected_public_key.append((char*)buffer);
    }
    
    // Compare it to public key
    std::ifstream public_key_file(public_key_path);
    std::string public_key((std::istreambuf_iterator<char>(public_key_file)), std::istreambuf_iterator<char>());
    public_key_file.close();

    return expected_public_key == public_key;
}

string get_type_of_user(const std::string &keyfile_name)
{
    string username = keyfile_name;
    username.erase(username.find_first_of("_ "));
    if (is_valid_keyfile(username)) {
        cout << "Logged in as " << username << endl;
        return username;
    }
    cout << "Invalid keyfile" << endl;
    exit(EXIT_FAILURE);
}

#endif // CMPT785_BIBIFI_USER_AUTHENTICATION_H
