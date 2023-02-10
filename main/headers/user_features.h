#ifndef CMPT785_BIBIFI_USER_FEATURES_H
#define CMPT785_BIBIFI_USER_FEATURES_H

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>

using namespace std;

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
  }

  // get command from the user
  string input_feature;
  getline(std::cin, input_feature);

  // get the first word (command) from the input
  string cmd, filename, username, directory_name, contents;
  istringstream iss(input_feature);
  iss >> cmd;

  if (cmd == "cd") {

    // TODO: support changing multiple directories at once (cd ../../dir1/dir2)
    iss >> directory_name;
    if (directory_name == "..") {
      chdir("..");
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

  // before exiting encrypt the filesystem again
  encrypt_filesystem();
  return 1;
}

#endif // CMPT785_BIBIFI_USER_FEATURES_H
