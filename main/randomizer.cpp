#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "headers/json.hpp"
using namespace std;
using json = nlohmann::json;

string Randomizer(int ch){
    // For randomised string generation
    char letters[26] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g',
                          'h', 'i', 'j', 'k', 'l', 'm', 'n',
                          'o', 'p', 'q', 'r', 's', 't', 'u',
                          'v', 'w', 'x', 'y', 'z' };
    string random_string = "";
    for (int i = 0; i<ch; i++)
        random_string = random_string + letters[rand() % 26];
    return random_string;
}

json get_metadata_json() {
    ifstream metadata("fs_metadata.json");
    json tree;
    metadata >> tree;
    metadata.close();
    return tree;
}

// get all nodes in a path
vector<string> splitPath(string str) {
    vector<string> tokens;
    string token;
    size_t pos = 0;
    while ((pos = str.find('/')) != string::npos) {
        token = str.substr(0, pos);
        if (!token.empty())
            tokens.push_back(token);
        str.erase(0, pos + 1);
    }
    tokens.push_back(str);
    return tokens;
}

// to be used in cd etc. to check if a path exists
bool is_valid_path(string username, string path) {
    json tree = get_metadata_json();
    string jpath = "/" + username + path;
    try {
        json::json_pointer ptr(jpath);
        json& val = tree.at(ptr);
        return true;
    } catch (const json::out_of_range& oor) {
        return false;
    }
}

// to be used in ls
vector<string> get_all_keys(string username, string path) {
    vector<string> keys;
    // check if path is valid
    if (!is_valid_path(username, path))
        return keys;

    json tree = get_metadata_json();
    // Get all keys at the given JPath
    string jpath = username + path;
    vector<string> path_nodes = splitPath(jpath);
    auto iter = path_nodes.begin();
    auto& node = tree;
    while (iter != path_nodes.end()) {
        node = node[*iter];
        iter++;
    }
    for (auto& el : node.items()) {
        keys.push_back(el.key());
    }
    return keys;
}

// return the random path for the PT path
string get_random_filePath (string username, string path) {
    if (!is_valid_path(username, path)){
        cout << "Invalid file/directory path" << endl;
        return "";
    }
    json tree = get_metadata_json();
    string r_filePath = username;
    auto t_node = tree[username];
    vector<string> path_nodes = splitPath(path);
    for (const string& str : path_nodes) {
        // if (t_node[str].is_null()){
        //     string r_node = Randomizer(10);
        //     if (is_dir)
        //         t_node[str] = json {{"rname", r_node}};
        //     else
        //         t_node[str] = r_node;
        // }
        if (t_node[str].is_object())
            r_filePath += "/" + (string)t_node[str]["rname"];
        else
            r_filePath += "/" + (string)t_node[str];
        t_node = t_node[str];
    }
    // ofstream metadata("fs_metadata.json");
    // metadata << tree.dump(4);
    // metadata.close();
    return r_filePath;
}

// only adds the last node of the path, expects rest of the path to be already present
string add_random_filePath (string username, string path, bool is_dir) {
    json tree = get_metadata_json();
    string r_filePath = username;
    try{

    }
    catch(exception e) {

    }
}

int main()
{
    string username = "uname1";
    // cout << boolalpha << is_valid_path(username, "/personal/dir1") << endl;
    // vector<string> test = get_all_keys(username, "/personal");
    // for (const string& str : test) {
    //     cout << str << endl;
    // }
    cout << get_random_filePath(username, "/personal/dir1/file1") << endl;
}
