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

using namespace std;
namespace fs = std::filesystem;

// TODO: once the filesystem directory and logic is created,
// TODO: use correct admin_root_path and user_root_path

// Set the new root directory here
fs::path admin_root_path = fs::current_path();
fs::path user_root_path = fs::current_path();

fs::path root_path;

string custom_pwd(string &filesystem_path) {
  string pwd = fs::current_path();
  pwd = pwd.erase(1, filesystem_path.length());
  return pwd;
}

int user_features(string user_name, User_type user_type, uint8_t key, string filesystem_path) {

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

    // also set root path to admin path which is the whole fs
    root_path = admin_root_path;
  } else if (user_type == user) {
    // set root path = user's root path which is its own directory
    root_path = user_root_path;
  }

  string input_feature, cmd, filename, username, directory_name, contents;

  do {

    // TODO: Replace "user_name" with actual user_name
    cout << user_name << " " << custom_pwd(filesystem_path) << "> ";
    // get command from the user
    getline(std::cin, input_feature);

    if (cin.eof()) {
        // Ctrl+D was pressed
        std::cout << "EOF detected." << std::endl;
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
        fs::file_status status = fs::status(entry_path);
        switch (status.type()) {
          case fs::file_type::directory: {
            cout << "d -> " << entry_path << endl;
            break;
          }
          case fs::file_type::regular: {
            cout << "f -> " << entry_path << endl; 
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
        system(("cat " + filename).c_str());
      }

    } else if (cmd == "share") {

      // TODO
      // `share <filename> <username>`

      /*
     * Share the file with the target user which should appear under the
     * `/shared` directory of the target user. The files are shared only with
     * read permission. The shared directory must be read-only. If the file
     * doesn't exist, print "File <filename> doesn't exist". If the user doesn't
     * exist, print "User <username> doesn't exist". First check will be on the
     * file.
     *
       */
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
                      system(("mkdir " + directory_name).c_str());

                    }

                  } else if (target_path == root_path.parent_path()) {

                    // like `going back one directory ..`
                    // create directory
                    system(("mkdir " + directory_name).c_str());

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
                        system(("mkdir " + directory_name).c_str());

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
                  system(("mkdir " + directory_name).c_str());

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

      // TODO
      //`mkfile <filename> <contents>`   -
      /*
     * Create a new file with the contents.
     * The contents will be printable ascii characters.
     * If a file with <filename> exists, it should replace the contents.
     * If the file was previously shared, the target user should see the new
     * contents of the file.
       */

      // TODO: delete this comment - A string is a group of characters with the last character being "\0".
      // TODO: should support multi-word and multi-line input
      istring_stream >> filename >> contents;



      filesystem::path path_obj(filename);
      string filename_str = path_obj.filename().string();
      string parent_path_str = path_obj.parent_path().string();

      // filename contains a path to the filename instead of just the filename
      if (!parent_path_str.empty()) {

        if (is_valid_path(parent_path_str, root_path)){

            if(is_valid_filename(filename)) {
              // TODO - replace the system call with encryption fn
              // create file
              cout << "try creating file.. " << endl;
              mkfile(filename, contents);
            } else {
              cerr << "not a valid filename, try again" << endl;
            }

        }

      } else if (!filename_str.empty()) {
        cout << "The file name is: " << filename_str << std::endl;

        if(is_valid_filename(filename)) {
            // TODO - replace the system call with encryption fn
            // create file
            cout << "try creating file.. " << endl;
            mkfile(filename, contents);
        } else {
            cerr << "not a valid filename, try again" << endl;
        }

      }



    } else if (cmd == "exit") {
      exit(EXIT_SUCCESS);

    } else if ((cmd == "adduser") && (user_type == admin)) {
        istring_stream >> filename;
        add_user(filename);

    } else {

      cout << "Invalid Command" << endl;

      // before exiting encrypt the filesystem again
      // encrypt_filesystem();
      // return 1;
    }

  } while (cmd != "exit"); // only exit out of command line when using "exit" cmd

  return 1;
}

#endif // CMPT785_BIBIFI_USER_FEATURES_H
