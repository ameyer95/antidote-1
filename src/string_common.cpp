#include "string_common.h"
#include <algorithm> // all_of, remove_if, transform, ...
#include <ctype.h> // for isspace
#include <locale> // for tolower
#include <sstream>
using namespace std;

void removeWhitespace(std::string &s) {
    // remove_if can't modify string length, isspace has overloads---see https://stackoverflow.com/a/83538
    s.erase(remove_if(s.begin(), s.end(), ::isspace), s.end());
}

bool allWhitespace(const string &s) {
    return all_of(s.begin(), s.end(), ::isspace);
}

void commaSplit(const string &s, vector<string> &store, bool trimWhitespace) {
    istringstream iss(s);
    string temp;
    while(iss.good()) {
        getline(iss, temp, ',');
        if(trimWhitespace) {
            removeWhitespace(temp);
        }
        store.push_back(temp);
    }
}

string toLowerCase(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower); // tolowercase
    return s;
}
