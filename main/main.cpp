#include <iostream>

#include "headers/user_authentication.h"
#include "headers/user_features.h"

using namespace std;

int main(int argc, char *argv[]) {

  // check for correct number arguments
  // ./fileserver counts as 1st argument, keyfile_name counts as 2nd argument
  if (argc != 2) {
    cout << "System accepts 1 argument initially. Please enter "
            "\"keyfile_name\" along with the program name"
         << endl;
  } else {

    string keyfile_name = argv[1];

    // TODO: check for user authentication
    string user_type = get_type_of_user(keyfile_name);
    if (user_type == "admin" || user_type == "user") {
      // user authenticated, allow "available commands" to be run
      user_features(user_type);
    } else {
      // Since the user wasn't authenticated, the login was failed and the
      // program was exited.
      cout << "Invalid keyfile" << endl;
      // before exiting encrypt the filesystem again
      encrypt_filesystem();
      exit(EXIT_FAILURE);
    }

    // before exiting encrypt the filesystem again
    encrypt_filesystem();
    return 1;
  }
}