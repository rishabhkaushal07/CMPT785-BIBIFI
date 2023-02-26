#ifndef CMPT785_BIBIFI_USER_FEATURES_H
#define CMPT785_BIBIFI_USER_FEATURES_H

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <filesystem>
#include <fstream>

#include "user_type.h"
#include "helper_functions.h"
#include "user_authentication.h"
#include "randomizer_function.h"

using namespace std;
namespace fs = std::filesystem;

// TODO: once the filesystem directory and logic is created,
// TODO: use correct admin_root_path and user_root_path

// Set the new root directory here
fs::path admin_root_path = fs::current_path() / "filesystem";
fs::path user_root_path = fs::current_path() / "filesystem";

fs::path root_path;

string custom_pwd(string &filesystem_path) {
  string pwd = fs::current_path();
  pwd = pwd.erase(1, filesystem_path.length());
  return pwd;
}

void add_contents_to_file(string filename, string filepath, string content) {
  filepath = filepath + "/" + filename;
  ofstream file(filepath, ios::app);
  if (fs::exists(filepath)) {
      file << content << std::endl;
      file.close();
  } else {
      ofstream new_file(filepath);
      new_file << content << endl;
      new_file.close();
  }
}

void share_file(vector<uint8_t> key, string username, string filename, string filesystem_path) {
  // check if file exists
  if (!fs::exists(filename)) {
      cout << "File does not exist." << endl;
      return;
  }

  // fetch user list and check is username exists
  string path = filesystem_path + "/public_keys";
  bool if_user_exists = false;
  for (fs::directory_entry entry : fs::directory_iterator(path)) {
    string entry_path = entry.path();
    entry_path.erase(entry_path.size() - 4);
    int delete_upto = entry_path.find_last_of('/') + 1;
    entry_path.erase(0, delete_upto);
    if (username == entry_path) {
      if_user_exists = true;
      break;
    }
  }
  if (!if_user_exists) {
    cout << "User " << username << " does not exist!" <<endl;
    return;
  }

  string content = decrypt_file(filename, key);
  string share_user_path = filesystem_path + "/filesystem/" + username + "/shared/" + filename;
  vector<uint8_t> share_key = read_enc_key_from_metadata(username, filesystem_path + "/metadata/");
  encrypt_file(share_user_path, content, share_key);

  string shared_data_path = filesystem_path + "/shared_files";
  add_contents_to_file(filename, shared_data_path, username);
}

void update_shared_files(vector<string> usernames, string filename, string filesystem_path, string content) {
  for (const string& username : usernames) {
    string share_user_path = filesystem_path + "/filesystem/" + username + "/shared/" + filename;
    vector<uint8_t> share_key = read_enc_key_from_metadata(username, filesystem_path + "/metadata/");
    encrypt_file(share_user_path, content, share_key);
  }
}

void check_if_shared(string filename, string filesystem_path, string content) {
  vector<string> usernames;
  string filepath = filesystem_path + "/shared_files/" + filename;
  if (fs::exists(filepath)) {
      ifstream file(filepath);
      string line;
      while (getline(file, line)) {
          usernames.push_back(line);
      }
      file.close();
  } 

  update_shared_files(usernames, filename, filesystem_path, content);
}

void make_directory(string directory_name, string &filesystem_path) {
  if (directory_name.find('/') != string::npos) {
    cout << "Directory name cannot contain /" << endl;
  } else {
    string path = custom_pwd(filesystem_path) + "/" + directory_name;
    string encrypted_name = encrypt_filename(path, filesystem_path);
    system(("mkdir " + encrypted_name).c_str());
  }
}

void make_file(string filename, string contents, vector<uint8_t> key, string filesystem_path) {
  string path = custom_pwd(filesystem_path) + "/" + filename;
  string encrypted_name = encrypt_filename(path, filesystem_path);
  encrypt_file(encrypted_name, contents, key);
  check_if_shared(encrypted_name, filesystem_path, contents);
}

int user_features(string user_name, User_type user_type, vector<uint8_t> key, string filesystem_path) {
  cout << "=======================" << endl;
  cout << "Available commands are: \n" << endl;

  cout << "cd <directory> \n"
          "pwd \n"
          "ls  \n"
          "cat <filename> \n"
          "share <filename> <username> \n"
          "mkdir <directory_name> \n"
          "mkfile <filename> <contents> \n"
          "exit \n";

  if (user_type == admin) {
    // if admin, allow the following command
    cout << "adduser <username>" << endl;
    cout << "=======================" << endl;

    // also set root path to admin path which is the whole fs
    root_path = admin_root_path;
  } else if (user_type == user) {
    cout << "=======================" << endl;
    // set root path = user's root path which is its own directory
    root_path = user_root_path / user_name;
  }

  fs::current_path(root_path);
  string input_feature, cmd, filename, username, directory_name, contents;

  do {
    cout << user_name << " " << custom_pwd(filesystem_path) << "> ";
    // get command from the user
    getline(std::cin, input_feature);

    if (cin.eof()) {
        // Ctrl+D was pressed
        std::cout << "Ctrl+D detected." << std::endl;
        return 1;
    }

    // get the first word (command) from the input
    istringstream istring_stream(input_feature);
    istring_stream >> cmd;

    if (cmd == "cd") {
      // get the directory name from istring stream buffer
      istring_stream >> directory_name;

      if(contains_backticks(directory_name)) {
        cerr << "Error: directory name should not contain `backticks`, try again." << endl;
      } else {
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
                  fs::current_path(root_path);
                } else if (target_path == root_path) {
                  if (current_path == root_path) {
                    // like `cd .`  - so no need to change the directory
                    fs::current_path(fs::current_path());
                  } else {
                    // go to root path
                    fs::current_path(root_path);
                  }
                } else if (target_path == root_path.parent_path()) {
                  // like `cd ..`
                  fs::current_path(fs::current_path().parent_path());
                } else {
                  // if the directory path is outside the root path
                  // Warn and stay in the current directory
                  cerr << "Directory is outside of the root directory." << endl;
                  cout << "Staying in current directory." << endl;
                }
              } else {
                if (target_path == root_path) {
                  if (current_path == root_path) {
                    // like `cd .`  - so no need to change the directory
                    fs::current_path(fs::current_path());
                  } else {
                    // go to root path
                    fs::current_path(root_path);
                  }
                } else {
                  // if the directory path is outside the root path
                  // Warn and stay in the current directory
                  cerr << "Directory is outside of the root directory." << endl;
                  cout << "Staying in current directory." << endl;
                }
              }
            } else {
              if (directory_name == "/") {
                // This should vary depending upon what kind of user is currently logged in
                // cd / should take you to the current user’s root directory
                fs::current_path(root_path);
              } else if (target_path == root_path) {
                if (current_path == root_path) {
                  // like `cd .`  - so no need to change the directory
                  fs::current_path(fs::current_path());
                } else {
                  // go to root path
                  fs::current_path(root_path);
                }
              } else if (target_path == root_path.parent_path()) {
                // like `cd ..`
                fs::current_path(fs::current_path().parent_path());
              } else if (fs::exists(directory_name) && fs::is_directory(directory_name)) {
                if (relative_path.has_parent_path()) {
                  if (relative_path.string().find("..") != string::npos) {
                    // if the directory path is outside the root path
                    // Warn and stay in the current directory
                    cerr << "Directory is outside of the root directory." << endl;
                    cout << "Staying in current directory." << endl;
                  } else {
                    // relative path is trying a subdirectory
                    if (fs::exists(directory_name) && fs::is_directory(directory_name)) {
                      // the directory exists, so we can change to given directory
                      fs::current_path(target_path);
                    } else {
                      // If a directory doesn't exist, the user should stay in the current directory
                      cerr << "Directory does not exist." << endl;
                      cout << "Staying in current directory." << endl;
                    }
                  }
                } else {
                  if (relative_path.string().find("..") != std::string::npos) {
                    // relative_path contains .. meaning it is trying to go outside root directory
                    // if the directory path is outside the root path
                    // Warn and stay in the current directory
                    cerr << "Directory is outside of the root directory." << endl;
                    cout << "Staying in current directory." << endl;
                  } else {
                    // the directory exists, so we can change to given directory
                    fs::current_path(target_path);
                  }
                }
              } else {
                // If a directory doesn't exist, the user should stay in the current directory
                cerr << "Directory does not exist." << endl;
                cout << "Staying in current directory." << endl;
              }
            }
          } else {
            // If a directory doesn't exist, the user should stay in the current directory
            cerr << "Directory does not exist." << endl;
            cout << "Staying in current directory." << endl;
          }
        } else {
          if (directory_name == "/"){
            // This should vary depending upon what kind of user is currently logged in
            // cd / should take you to the current user’s root directory
            fs::current_path(root_path);
          } else {
            // if the directory path is outside the root path
            // Warn and stay in the current directory
            cerr << "Directory is outside of the root directory." << endl;
            cout << "Staying in current directory." << endl;
          }
        }
      }
    } else if (cmd == "pwd") {
      string pwd = custom_pwd(filesystem_path);
      cout << pwd << endl;
    } else if (cmd == "ls") {
      string path = fs::current_path();

      cout << "d -> ." << endl;
      if (path != filesystem_path + "/filesystem") {
        cout << "d -> .." << endl;
      }

      for (fs::directory_entry entry : fs::directory_iterator(path)) {
        string entry_path = entry.path();
        int delete_upto = entry_path.find_last_of('/') + 1;
        entry_path.erase(0, delete_upto);

        //check if the file starts with .
        if (entry_path.find(".") == 0) {
          continue;
        }

        fs::file_status status = fs::status(entry_path);
        cout << " file = " << entry_path << endl;
        string decrypted_name = decrypt_filename(entry_path, filesystem_path);
        switch (status.type()) {
          case fs::file_type::directory: {
            cout << "d -> " << decrypted_name << endl;
            break;
          }
          case fs::file_type::regular: {
            cout << "f -> " << decrypted_name << endl; 
            break;
          }
          default: break;
        }
      }
    } else if (cmd == "cat") {
      istring_stream >> filename;
      if (filename.empty()) {
        cout << "filename not provided";
      } else {
        // string path = custom_pwd(filesystem_path) + "/" + filename;
        cout << decrypt_file(filename, key) << endl;
        // system(("cat " + filename).c_str());
      }
    } else if (cmd == "share") {
      string share_username;
      istring_stream >> filename;
      istring_stream >> share_username;

      share_file(key, share_username, filename, filesystem_path);
    } else if (cmd == "mkdir") {
      // create a new directory
      istring_stream >> directory_name;

      if(contains_backticks(directory_name)) {
        cerr << "Error: directory name should not contain `backticks`, try again." << endl;
      } else {
        // construct a target (absolute) path from the directory name
        fs::path current_path = fs::current_path();
        fs::path target_path = fs::absolute(directory_name);
        fs::path relative_path = fs::relative(target_path, root_path);
        fs::path resolved_root = fs::absolute(root_path);
        fs::path resolved_target = fs::absolute(target_path);

        if (directory_name.empty()) {
            cout << "directory_name not provided";
        } else if (directory_name == "." || directory_name == "..") {
            // . and .. directories always exist - try `ls -alh` to see all the dirs
            cerr << "Directory already exists." << endl;
        } else {
            if (target_path.has_relative_path()) {
              if (fs::exists(directory_name) && fs::is_directory(directory_name)) {
                // If a directory with this name exists, print "Directory already exists"
                cerr << "Directory already exists." << endl;
              } else {
                if(target_path.lexically_relative(root_path).native().front() == '.') {
                  if(directory_name == "." || directory_name == "..") {
                    if (target_path == root_path) {
                      if (current_path == root_path) {
                        // like `mkdir .`  - so no need to create the directory
                        cerr << "Directory already exists." << endl;
                      } else {
                        // create directory
                        make_directory(directory_name, filesystem_path);
                        // system(("mkdir " + directory_name).c_str());
                      }
                    } else if (target_path == root_path.parent_path()) {
                      // like `going back one directory ..`
                      // create directory
                      make_directory(directory_name, filesystem_path);
                      // system(("mkdir " + directory_name).c_str());
                    } else {
                      // if the directory path is outside the root path
                      // Warn and stay in the current directory
                      cerr << "Directory is outside of the root directory." << endl;
                    }
                  } else {
                    if (target_path == root_path) {
                      if (current_path == root_path) {
                        // like `mkdir .`  - so no need to change the directory
                        cerr << "Directory already exists." << endl;
                      } else {
                        // like creating root path's dir
                        cerr << "Directory already exists." << endl;
                      }
                    } else {
                      // if the directory path is outside the root path
                      // Warn and stay in the current directory
                      cerr << "Directory is outside of the root directory." << endl;
                    }
                  }
                } else {
                  if (directory_name == "/") {
                    // mkdir /  means creating current user’s root directory
                    // but this should already exist so error out
                    cerr << "Directory already exists." << endl;
                  } else if (target_path == root_path) {
                    if (current_path == root_path) {
                      // like `mkdir .`
                      cerr << "Directory already exists." << endl;
                    } else {
                      // target path going to root path
                      // but this should already exist so error out
                      cerr << "Directory already exists." << endl;
                    }
                  } else if (target_path == root_path.parent_path()) {
                    // like `mkdir ..`
                    cerr << "Directory already exists." << endl;
                  } else if (fs::exists(directory_name) && fs::is_directory(directory_name)) {
                    if (relative_path.has_parent_path()) {
                      // if the directory path is outside the root path
                      // Warn and stay in the current directory
                      cerr << "Directory is outside of the root directory." << endl;

                      if (relative_path.string().find("..") != string::npos) {
                        // if the directory path is outside the root path
                        // Warn and stay in the current directory
                        cerr << "Directory is outside of the root directory." << endl;
                      } else {
                        // relative path is trying a subdirectory
                        if (fs::exists(directory_name) && fs::is_directory(directory_name)) {
                          // the directory exists, so we shouldn't create it
                          cerr << "Directory already exists." << endl;
                        } else {
                          // If a directory doesn't exist,
                          // then check if there exists its parent directory
                          // if parent directory exists it's okay to create the given directory
                          // else do not create it
                          // good thing is that system() automatically checks this
                          // so no need to explicitly check for it
                          make_directory(directory_name, filesystem_path);
                          // system(("mkdir " + directory_name).c_str());
                        }
                      }
                    } else {
                      if (relative_path.string().find("..") != std::string::npos) {
                        // relative_path contains .. meaning it is trying to go outside root directory
                        // if the directory path is outside the root path
                        // Warn and stay in the current directory
                        cerr << "Directory is outside of the root directory." << endl;
                      } else {
                        // the directory exists, can't create it
                        cerr << "Directory already exists." << endl;
                      }
                    }
                  } else {
                    // directory doesn't exist, so create it
                    make_directory(directory_name, filesystem_path);
                    // system(("mkdir " + directory_name).c_str());
                  }
                }
              }
            } else {
              if (directory_name == "/"){
                // This should vary depending upon what kind of user is currently logged in
                // mkdir /  means creating current user’s root directory
                // but this should already exist so error out
                cerr << "Directory already exists." << endl;
              } else {
                // if the directory path is outside the root path
                // Warn and stay in the current directory
                cerr << "Directory is outside of the root directory." << endl;
              }
            }
        }
      }
    } else if (cmd == "mkfile") {
      //`mkfile <filename> <contents>`
      istring_stream >> filename;
      getline(istring_stream, contents);

      filesystem::path path_obj(filename);
      string filename_str = path_obj.filename().string();
      string parent_path_str = path_obj.parent_path().string();
      string path = custom_pwd(filesystem_path) + "/" + filename;

      // filename contains a path to the filename instead of just the filename
      if (!parent_path_str.empty()) {

        if (is_valid_path(parent_path_str, root_path)){
            if(is_valid_filename(filename)) {
              // create file
              make_file(filename, contents, key, filesystem_path);
              // encrypt_file(filename, contents, key);
              // check_if_shared(filename, filesystem_path, contents);
            } else {
              cerr << "not a valid filename, try again" << endl;
            }
        }
      } else if (!filename_str.empty()) {
        if(is_valid_filename(filename)) {
            // create file
            make_file(filename, contents, key, filesystem_path);
            // encrypt_file(filename, contents, key);
            // check_if_shared(filename, filesystem_path, contents);
        } else {
            cerr << "not a valid filename, try again" << endl;
        }
      }
    } else if (cmd == "exit") {
      exit(EXIT_SUCCESS);
    } else if ((cmd == "adduser") && (user_type == admin)) {
        string new_user;
        istring_stream >> new_user;
        add_user(new_user, filesystem_path, false);
    } else {
      cout << "Invalid Command" << endl;
    }
  } while (cmd != "exit"); // only exit out of command line when using "exit" cmd

  return 1;
}

#endif // CMPT785_BIBIFI_USER_FEATURES_H
