#include <iostream>
#include <fstream>
#include "headers/json.hpp"
using namespace std;
using json = nlohmann::json;
int main()
{
std::ifstream f("data.json");
json data = json::parse(f);
// Access the values existing in JSON data
string name = data.value("test", "not found");
// Access a value that does not exist in the JSON data
string check = data.value("test2", "not found");
// Print the values
cout << "Test: " << name << endl;
cout << "Check: " << check << endl;
return 0;
}