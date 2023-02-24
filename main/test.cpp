#include <iostream>
#include <fstream>
#include "headers/json.hpp"
using namespace std;
using json = nlohmann::json;

int main() {
    std::ifstream f("metadata.json");
    json jsonfile = json::parse(f);

    jsonfile.push_back({"check","123"});

    std::ofstream file("metadata.json");
    file << jsonfile;
}