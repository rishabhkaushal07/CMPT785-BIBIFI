// user authentication

#ifndef CMPT785_BIBIFI_USER_AUTHENTICATION_H
#define CMPT785_BIBIFI_USER_AUTHENTICATION_H

#endif // CMPT785_BIBIFI_USER_AUTHENTICATION_H

#include "encryption.h"
#include <string>
#include <openssl/sha.h>

using namespace std;

bool is_valid_keyfile(const string &keyfile_name);
  std::ifstream keyfile(keyfile_name);
  if (!keyfile) {
    std::cerr << "Error opening keyfile: " << keyfile_name << std::endl;
    return false;
  }
  std::string keyfile_contents((std::istreambuf_iterator<char>(keyfile)),
                               std::istreambuf_iterator<char>());
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, keyfile_contents.c_str(), keyfile_contents.length());
  SHA256_Final(hash, &sha256);
  std::string computed_hash = std::string((char *)hash, SHA256_DIGEST_LENGTH);
  // Compare computed_hash with expected_hash
  if (computed_hash == expected_hash) {
    return true;
  } else {
    return false;
  }
}
# Note: This is just a sample code for validation and does not include the actual expected hash value, which should be stored securely in a separate location.

string get_type_of_user(const std::string &keyfile_name) {

  // TODO: first check if the keyfile is a valid file for or not
  if (is_valid_keyfile(keyfile_name)) {

    // after authenticating that the keyfile is valid and appropriate
    // TODO: Decrypt the filesystem and,
    decrypt_filesystem();

    // TODO: return the type of the user based on the keyfile
    string username;
    if (keyfile_name == "admin_keyfile_name") {
      username = "admin";
    } else {
      username = "user";
    }

    cout << "Logged in as " << username << endl;
    return username;
  }

  // Since the user wasn't authenticated, the login was failed and the program
  // was exited.
  cout << "Invalid keyfile" << endl;
  // before exiting encrypt the filesystem again
  encrypt_filesystem();
  exit(EXIT_FAILURE);
}

// TODO: Implement authentication, currently it just checks that keyfile should
// not be empty
bool is_valid_keyfile(const string &keyfile_name) {
  if (!keyfile_name.empty()) {
    return true;
  }

  // before exiting encrypt the filesystem again
  encrypt_filesystem();
  return false;
}
