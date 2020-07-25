#include "ArffScanner.h"
#include "Error.h"

#include <algorithm>

using namespace std;

static inline void ltrim(string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [](int ch) {
        return !isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), [](int ch) {
        return !isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(string &s) {
    ltrim(s);
    rtrim(s);
}

ArffScanner::ArffScanner(const string& _file): file(_file),
                                            fp(NULL),
                                            lineNum(0),
                                            isEOF(false) {
    fp = ifstream(file);
    err_handler = new Error(); 
    if(!fp.is_open()) { 
        err_handler->fatal("Cannot open specified arff file");
    }
}

ArffScanner::~ArffScanner() {
    if(fp.is_open()) {
        fp.close();
    }
    delete err_handler; 
}

bool ArffScanner::nextLine() {
    if(!getline(fp, curLine)) 
        isEOF = true;
    else {
        trim(curLine);
        if(curLine.length() == 0 || curLine.at(0) == '%') {
            lineNum++; 
            nextLine();
        }
        iss = istringstream(curLine);
    }
    lineNum++; 
    return isEOF;
}

string ArffScanner::nextWord() {
    string ret; 
    iss >> skipws >> ret;
    if(ret.at(0) == '"') {
        while(ret.at(ret.size() - 1) != '"') {
            string tmp; 
            iss >> tmp; 
            ret = ret + " " + tmp; 
        }
    }
    return ret;  
}
