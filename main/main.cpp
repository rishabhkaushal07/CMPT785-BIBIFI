#include <iostream>
#include <sys/stat.h>

#include "headers/user_type.h"
#include "headers/user_authentication.h"
#include "headers/user_features.h"

using namespace std;

int main(int argc, char *argv[]) {
  // check for correct number arguments
  // ./fileserver counts as 1st argument, keyfile_name counts as 2nd argument
  struct stat sb;
  mode_t mode = 0666;
  if(stat("filesystem", &sb) == 0) {
    if(argc != 2) {
      cout << "System accepts 1 argument initially. Please enter "
              "\"keyfile_name\" along with the program name"
          << endl;
      return 1;
    }
    else{
      string keyfile_name = argv[1];
      string filesystem_path = fs::current_path();
      // user authenticated, allow "available commands" to be run
      string user_name = get_type_of_user(keyfile_name);
      User_type user_type;
      if(user_name == "admin")
        user_type = admin;
      else
        user_type = user;

      // read user's enc key from metadata file
      uint8_t key = read_enc_key_from_metadata(user_name);
      user_features(user_name, user_type, key, filesystem_path);
    }
  }
  else{
    mode_t old_umask = umask(0); // to ensure the following modes get set
    if (mkdir("filesystem", mode) != 0) {
      std::cerr << "Error creating filesystem." << std::endl;
      return 1;
    }
    if (mkdir("public_keys", mode) != 0) {
      std::cerr << "Error creating public_keys." << std::endl;
      return 1;
    }
    if (mkdir("private_keys", mode) != 0) {
      std::cerr << "Error creating private_keys." << std::endl;
      return 1;
    }
    if (mkdir("metadata", mode) != 0) {
      std::cerr << "Error creating metadata directory." << std::endl;
      return 1;
    }
    umask(old_umask); // Restore the original umask value
    add_user("admin",true);
  }
}
