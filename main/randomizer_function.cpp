#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "headers/json.hpp"
using namespace std;
using json = nlohmann::json;

// string Randomizer(int ch);
// json read_metadata_json(void);
// string get_filename(string randomized_name);
// string get_randomized_name(string filename);
// string get_randomized_file_path(string filepath);
// string get_plaintext_file_path(string randomized_filepath);
// string encrypt_filename(string filename);
// string decrypt_filename(string randomized_name);

string Randomizer(int ch) {
    srand(time(NULL));
    char letters[26] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g',
                          'h', 'i', 'j', 'k', 'l', 'm', 'n',
                          'o', 'p', 'q', 'r', 's', 't', 'u',
                          'v', 'w', 'x', 'y', 'z' };
    string random_string = "";
    for (int i = 0; i<ch; i++)
        random_string = random_string + letters[rand() % 26];
    return random_string;
}

json read_metadata_json(){
    ifstream metadata_file("metadata.json");
    json metadata_json = json::parse(metadata_file);
    return metadata_json;
}

string get_filename(string randomized_filename){
    // Parsing JSON object
    ifstream metadata_file("metadata.json");
    json metadata_json = json::parse(metadata_file);
    // Fetching randomized_filename filepath mapping
    string filename = metadata_json[randomized_filename];
    // TODO: error handling
    // if (filename.empty())
    //     handleErrors();
    return filename;
}

string get_randomized_name(string filename){
    // Create a JSON object
    json obj = read_metadata_json();
    string randomized_name;

    // Iterate over the JSON object and check the value of each key
    for (auto itr = obj.begin(); itr != obj.end(); ++itr) {
        if (itr.value() == filename) {
            randomized_name = itr.key();
            break;
        }
    }
    return randomized_name;
}

string get_randomized_file_path(string filepath){
    char separator = '/';
    int i = 0;
    string randomized_path = "";
    // Temporary string used to split the string.
    string s,temp; 
    while (filepath[i] != '\0') {
        if (filepath[i] != separator) {
            // Append the char to the temp string.
            s += filepath[i]; 
        } else {
            temp = get_randomized_name(s);
            randomized_path = randomized_path+ "/" + temp;
            s.clear();
        }
        i++;
    }
    // Output the last stored word.
    temp = get_randomized_name(s);
    randomized_path = randomized_path+ "/" + temp;
    return randomized_path;
}

string get_plaintext_file_path(string randomized_filepath){
    char separator = '/';
    int i = 0;
    string plaintext_path = "";
    // Temporary string used to split the string.
    string s,temp; 
    while (randomized_filepath[i] != '\0') {
        if (randomized_filepath[i] != separator) {
            // Append the char to the temp string.
            s += randomized_filepath[i]; 
        } else {
            temp = get_filename(s);
            plaintext_path = plaintext_path + "/" + temp;
            s.clear();
        }
        i++;
    }
    // Output the last stored word.
    temp = get_filename(s);
    plaintext_path = plaintext_path + "/" + s;
    return plaintext_path;
}

string encrypt_filename(string filename){
    //Generating the random string for filename
    string randomized_filename = Randomizer(10);
    //Reading the metadata JSON for inserting the randomizer-filename mapping
    ifstream metadata_file("metadata.json");
    json metadata_json = json::parse(metadata_file);
    json input_json = json::object();
    input_json[randomized_filename] = filename;

    //Inserting the new mapping into the metadata JSON file
    metadata_json.update(input_json.begin(), input_json.end(), true);
    ofstream file("metadata.json");

    //Writing into the metadata JSON file and returning the random string
    file << metadata_json;
    return randomized_filename;
}

string decrypt_filename(string randomized_filename){
    string filename;
    //Fetching the filename from the metadata JSON file and returning the filename
    filename = get_filename(randomized_filename);
    return filename;
}

int main()
{
    string filename = "testfile.txt";
    string random_name = encrypt_filename(filename);
    cout << "Filename_encryption: " << random_name << endl;
    cout << "Filename_decryption: " << decrypt_filename(random_name) << endl;
    cout << "Get_filename: " << get_filename(random_name) << endl;
    cout << "Get_random_name: " << get_randomized_name(filename) << endl;
    encrypt_filename("dir1");
    encrypt_filename("dir2");
    string filepath = "dir1/dir2/testfile.txt";
    string random_fp = get_randomized_file_path(filepath);
    cout << "Randomized path: " << random_fp << endl;
    cout << "PT_path: " << get_plaintext_file_path(random_fp) << endl;
    return 0;
}