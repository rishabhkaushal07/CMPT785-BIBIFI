#include <iostream>
#include <fstream>
#include "headers/json.hpp"
using namespace std;
using json = nlohmann::json;

int main() {
    std::ifstream f("metadata.json");
    json jsonfile = json::parse(f);
    json insertVal = R"( {"check":"123"} )"_json;
    cout << jsonfile;
    jsonfile.update(insertVal.begin(), insertVal.end(), true);
    std::ofstream file("metadata.json");
    file << jsonfile;
}