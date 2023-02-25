#ifndef CMPT785_BIBIFI_HELPER_FUNCTIONS_H
#define CMPT785_BIBIFI_HELPER_FUNCTIONS_H

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <filesystem>
#include <fstream>
#include <openssl/rand.h>
#include <regex>
#include "enc_consts.h"

using namespace std;
namespace fs = std::filesystem;

// TODO: once the filesystem directory and logic is created,
// TODO: use correct admin_root_path and user_root_path

// TODO: delete this function once the mkfile cmd starts to use encrypt
void mkfile(std::string filename, std::string contents) {

  ofstream file;
  file.open(filename);
  if (file.is_open()) {
    file << contents;
    cout << "Success: File created successfully!" << endl;
    file.close();
  } else {
    cerr << "Error: Could not create file!" << endl;
  }

}

// Here, we first check if the input string has more than one character.
// If it does, we remove all consecutive / characters using the unique() algorithm from the algorithm header.
// We then erase the characters that follow the last non / character.
// If the input string contains only / characters,
// we simply return a string with a single / character.
string normalize_path(string path) {
  int n = path.length();
  int i = n - 1;
  while (i > 0 && path[i] == '/') {
    i--;
  }
  path.erase(i+1, n-i-1);
  if (path.length() > 1) {
    string::iterator it = unique(path.begin(), path.end(), [](char currentChar, char nextChar) {
      return currentChar == nextChar && currentChar == '/';
    });
    path.erase(it, path.end());
  }
  return path;
}

bool is_valid_path(string &directory_name, const fs::path& root_path) {
  directory_name = normalize_path(directory_name);

  // construct a target (absolute) path from the directory name
  fs::path current_path = fs::current_path();
  fs::path target_path = fs::absolute(directory_name);
  fs::path relative_path = fs::relative(target_path, root_path);
  fs::path resolved_root = fs::absolute(root_path);
  fs::path resolved_target = fs::absolute(target_path);


  if (target_path.has_relative_path()) {

    if (fs::exists(directory_name) && fs::is_directory(directory_name)) {

      // checking this before because lexically_relative errors if the dir doesn't exist
      if(target_path.lexically_relative(root_path).native().front() == '.') {

        if(directory_name == "." || directory_name == "..") {

          if (directory_name == "/") {

            // This should vary depending upon what kind of user is currently logged in
            // cd / should take you to the current user’s root directory
            // this is a valid directory so return true
            return true;

          } else if (target_path == root_path) {

            if (current_path == root_path) {

              // like `cd .`
              // currently directory is a valid directory so return true
              return true;

            } else {

              // like going to root path
              // valid directory so return true
              return true;

            }

          } else if (target_path == root_path.parent_path()) {

            // like `cd ..`
            // valid directory so return true
            return true;

          } else {

            // if the directory path is outside the root path
            // Warn and return false
            cerr << "Path is outside of the root directory. " << endl;
            return false;

          }

        } else {

          if (target_path == root_path) {

            if (current_path == root_path) {

              // like `cd .`
              // valid directory so return true
              return true;

            } else {

              // going to the root path
              // valid directory so return true
              return true;

            }

          } else {

            // if the directory path is outside the root path
            // Warn and return false
            cerr << "Path is outside of the root directory. " << endl;
            return false;

          }

        }

      } else {

        if (directory_name == "/") {

          // This should vary depending upon what kind of user is currently logged in
          // like cd /
          // valid directory so return true
          return true;

        } else if (target_path == root_path) {

          if (current_path == root_path) {

            // like `cd .`
            // valid directory so return true
            return true;

          } else {

            // like going to root path
            // valid directory so return true
            return true;

          }

        } else if (target_path == root_path.parent_path()) {

          // like `cd ..`
          // valid directory so return true
          return true;

        } else if (fs::exists(directory_name) && fs::is_directory(directory_name)) {

          if (relative_path.has_parent_path()) {

            if (relative_path.string().find("..") != string::npos) {

              // if the directory path is outside the root path
              // Warn and return false
              cerr << "Path is outside of the root directory." << endl;
              return false;

            } else {

              // relative path is trying a subdirectory
              if (fs::exists(directory_name) && fs::is_directory(directory_name)) {

                // if directory exists, then it's an okay path
                return true;

              } else {

                // If a directory doesn't exist, the user should stay in the current directory
                cerr << "Directory in the path does not exist." << endl;
                cout << "Staying in current directory." << endl;
                return false;

              }

            }

          } else {

            if (relative_path.string().find("..") != std::string::npos) {

              // relative_path contains .. meaning it is trying to go outside root directory
              // if the directory path is outside the root path
              // Warn and return false
              cerr << "Path is outside of the root directory. " << endl;
              return false;

            } else {

              // the directory exists
              // valid directory so return true
              return true;

            }

          }

        } else {

          // If a directory doesn't exist, the user should stay in the current directory
          cerr << "Directory in the path does not exist." << endl;
          cout << "Staying in current directory." << endl;
          return false;

        }

      }

    } else {

      // If a directory doesn't exist, the user should stay in the current directory
      cerr << "Directory in the path does not exist." << endl;
      cout << "Staying in current directory." << endl;
      return false ;

    }

  } else {

    if (directory_name == "/"){

      // This should vary depending upon what kind of user is currently logged in
      // like cd /
      // valid directory so return true
      return true;

    } else {

      // if the directory path is outside the root path
      // Warn and return false
      cerr << "Path is outside of the root directory " << endl;
      return false;

    }

  }

  // return false in the end as a failsafe
  return false;

}

void add_enc_key_to_metadata(string username){
    // create metadata key file if not present
    fstream file("metadata/" + username + "_key", ios::out | ios::binary);
    if (!file.is_open()) {
        std::cout << "Failed to create user metadata key file" << std::endl;
        return;
    }
    // create 256-bit key 
    uint8_t key[KEY_SIZE];
    RAND_bytes(key, KEY_SIZE);
    file.write((char*)key, KEY_SIZE);
    file.close();
}

uint8_t* read_enc_key_from_metadata(string username){
    fstream file("metadata/" + username + "_key", ios::in | ios::binary);
    if (!file.is_open()) {
      std::cout << "Failed to read key from metadata" << std::endl;
    }
    uint8_t key[KEY_SIZE];
    file.read((char*)key, KEY_SIZE);
    return key;
}

bool contains_backticks(const string& input) {

  cout << input<<endl;
  if (input.find('`') == std::string::npos) {
    // `backtick` found
    return false;
  }

  // `backtick` not found
  return true;
}

bool is_valid_filename(const string& filename) {

  // reference: https://stackoverflow.com/questions/11794144/regular-expression-for-valid-filename
  regex pattern("^[a-zA-Z0-9](?:[a-zA-Z0-9 ._-]*[a-zA-Z0-9])?\\.[a-zA-Z0-9_-]+$");

  int max_length = 255;

  // the filename matches pattern and is less than max_length, then return true
  if((regex_match(filename, pattern)) && (filename.length() < max_length)) {
    return true;
  }

  return false;

}

void create_init_fs_for_user(string username) {
  mode_t old_umask = umask(0); // to ensure the following modes get set
  mode_t mode = 0700;
  string u_folder = "filesystem/" + username;
  if (mkdir(u_folder.c_str(), mode) != 0) {
    std::cerr << "Error creating root folder for " << username << std::endl;
  }
  else {
    u_folder = "filesystem/" + username + "/personal";
    if (mkdir(u_folder.c_str(), mode) != 0) {
      std::cerr << "Error creating personal folder for " << username << std::endl;
    }
    u_folder = "filesystem/" + username + "/shared";
    if (mkdir(u_folder.c_str(), mode) != 0) {
      std::cerr << "Error creating shared folder for " << username << std::endl;
    }
  }
  umask(old_umask); // Restore the original umask value
}

#endif // CMPT785_BIBIFI_HELPER_FUNCTIONS_H
