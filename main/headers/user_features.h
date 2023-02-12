#ifndef CMPT785_BIBIFI_USER_FEATURES_H
#define CMPT785_BIBIFI_USER_FEATURES_H

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

// TODO: once the filesystem directory and logic is created,
// TODO: use correct admin_root_path and user_root_path

// Set the new root directory here
fs::path admin_root_path = fs::current_path();
fs::path user_root_path = fs::current_path();

fs::path root_path;

// Here, we first check if the input string has more than one character.
// If it does, we remove all consecutive / characters using the unique() algorithm from the algorithm header.
// We then erase the characters that follow the last non / character.
// If the input string contains only / characters,
// we simply return a string with a single / character.
string normalize_path(std::string path) {
  int n = path.length();
  int i = n - 1;
  while (i > 0 && path[i] == '/') {
    i--;
  }
  path.erase(i+1, n-i-1);
  if (path.length() > 1) {
    std::string::iterator it = unique(path.begin(), path.end(), [](char a, char b) {
      return a == b && a == '/';
    });
    path.erase(it, path.end());
  }
  return path;
}

int user_features(string user_type) {

  cout << "Available commands are: \n" << endl;

  cout << "cd <directory> \n"
          "pwd \n"
          "ls  \n"
          "cat <filename> \n"
          "share <filename> <username> \n"
          "mkdir <directory_name> \n"
          "mkfile <filename> <contents> \n"
          "exit \n";

  if (user_type == "admin") {
    // if admin, allow the following command
    cout << "adduser <username>" << endl;

    // also set root path to admin path which is the whole fs
    root_path = admin_root_path;
  } else if (user_type == "user") {
    // set root path = user's root path which is its own directory
    root_path = user_root_path;
  }

  string input_feature, cmd, filename, username, directory_name, contents;

  do {
    // get command from the user
    getline(std::cin, input_feature);

    // get the first word (command) from the input
    istringstream iss(input_feature);
    iss >> cmd;

    if (cmd == "cd") {

      // get the directory name from istring stream buffer
      iss >> directory_name;

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
                cerr << "Directory " << target_path << " is outside of the root directory " << root_path << endl;
                cout << "Staying in current directory: " << fs::current_path() << std::endl;
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
                cerr << "Directory " << target_path << " is outside of the root directory " << root_path << endl;
                cout << "Staying in current directory: " << fs::current_path() << std::endl;
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

                // if the directory path is outside the root path
                // Warn and stay in the current directory
                cerr << "Directory " << target_path << " is outside of the root directory " << root_path << endl;
                cout << "Staying in current directory: " << fs::current_path() << std::endl;
              } else {

                if (relative_path.string().find("..") != std::string::npos) {

                  // relative_path contains .. meaning it is trying to go outside root directory
                  // if the directory path is outside the root path
                  // Warn and stay in the current directory
                  cerr << "Directory " << target_path << " is outside of the root directory " << root_path << endl;
                  cout << "Staying in current directory: " << fs::current_path() << std::endl;
                } else {

                  // the directory exists, so we can change to given directory
                  fs::current_path(target_path);
                }

              }

            } else {

              // If a directory doesn't exist, the user should stay in the current directory
              cerr << "Directory " << target_path << " does not exist." << endl;
              cout << "Staying in current directory:" << fs::current_path() << std::endl;
            }

          }

        } else {

          // If a directory doesn't exist, the user should stay in the current directory
          cerr << "Directory " << target_path << " does not exist." << endl;
          cout << "Staying in current directory:" << fs::current_path() << std::endl;
        }

      } else {

        if (directory_name == "/"){

          // This should vary depending upon what kind of user is currently logged in
          // cd / should take you to the current user’s root directory
          fs::current_path(root_path);
        } else {

          // if the directory path is outside the root path
          // Warn and stay in the current directory
          cerr << "Directory " << target_path << " is outside of the root directory " << root_path << endl;
          cout << "Staying in current directory: " << fs::current_path() << std::endl;
        }

      }


    } else if (cmd == "pwd") {

      system("pwd");

    } else if (cmd == "ls") {

      system("ls");

    } else if (cmd == "cat") {

      iss >> filename;
      if (filename.empty()) {
        cout << "filename not provided";

        // before exiting encrypt the filesystem again
        encrypt_filesystem();
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
      iss >> directory_name;

      if (directory_name.empty()) {
        cout << "directory_name not provided";
      } else {
        system(("mkdir " + directory_name).c_str());
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
      iss >> filename >> contents;

      string cat = "echo " + contents + " > " + filename;
      system(cat.c_str());

    } else if (cmd == "exit") {

      // before exiting encrypt the filesystem again
      encrypt_filesystem();
      exit(EXIT_FAILURE);

    } else if ((cmd == "adduser") && (user_type == "admin")) {

      // TODO
      /*
     *
     * `adduser <username>`
     * This command should create a keyfile called username_keyfile
     * on the host which will be used by the user to access the filesystem.
     * If a user with this name already exists, print "User <username> already
     * exists"
       */

    } else {

      cout << "Invalid Command" << endl;

      // before exiting encrypt the filesystem again
      encrypt_filesystem();
      return 1;
    }

  } while (cmd != "exit"); // only exit out of command line when using "exit" cmd

  // before exiting encrypt the filesystem again
  encrypt_filesystem();
  return 1;
}

#endif // CMPT785_BIBIFI_USER_FEATURES_H
