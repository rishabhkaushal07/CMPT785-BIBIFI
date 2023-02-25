#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "headers/json.hpp"
using namespace std;
using json = nlohmann::json;

string Randomizer(int ch);
json read_metadata_json(void);
string get_filename(string randomized_name);
string get_randomized_name(string filename);
string get_randomized_file_path(string filepath);
string get_plaintext_file_path(string randomized_filepath);
string encrypt_filename(string filename);
string decrypt_filename(string randomized_name);

std::string Randomizer(int ch) {
    // For randomised string generation
    const std::string letters = "abcdefghijklmnopqrstuvwxyz";
    std::stringstream random_string;
    srand(static_cast<unsigned int>(time(nullptr)));
    for (int i = 0; i < ch; ++i) {
        random_string << letters[std::rand() % letters.length()];
    }
    return random_string.str();
}

json read_metadata_json(){
    std::ifstream metadata_file("metadata.json");
    json metadata_json = json::parse(metadata_file);
    return metadata_json;
}

std::string get_filename(string randomized_filename){
    // Parsing JSON object
    std::ifstream metadata_file("metadata.json");
    json metadata_json = json::parse(metadata_file);
    // Fetching randomized_filename filepath mapping
    std::string filename = metadata_json[randomized_filename];
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
    srand(time(NULL));
    //Generating the random string for filename
    string randomized_filename = Randomizer(10);
    //Reading the metadata JSON for inserting the randomizer-filename mapping
    std::ifstream metadata_file("metadata.json");
    json metadata_json = json::parse(metadata_file);
    json input_json = json::object();
    input_json[randomized_filename] = filename;

    //Inserting the new mapping into the metadata JSON file
    metadata_json.update(input_json.begin(), input_json.end(), true);
    std::ofstream file("metadata.json");

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
    cout << "Filename_encryption: " << random_name << std::endl;
    cout << "Filename_decryption: " << decrypt_filename(random_name) << std::endl;
    cout << "Get_filename: " << get_filename(random_name) << std::endl;
    cout << "Get_random_name: " << get_randomized_name(filename) << std::endl;
    encrypt_filename("dir1");
    encrypt_filename("dir2");
    string filepath = "dir1/dir2/testfile.txt";
    cout << "Randomized path: " << get_randomized_file_path(filepath) << std::endl;
    cout << "PT_path: " << get_plaintext_file_path() << std::endl;
    return 0;
}