#include <iostream>
#include <fstream>
#include "headers/json.hpp"
using namespace std;
using json = nlohmann::json;

int main() {
    json jsonfile;

    jsonfile["check"] = "123";

    std::ofstream file("metadata.json");
    file << jsonfile;
}