#ifndef ARFFSCANNER_H
#define ARFFSCANNER_H

#include "Error.h"

#include <string> 
#include <iostream> 
#include <fstream>
#include <sstream>

using namespace std;

class ArffScanner {
private:
    string file;
    ifstream fp;
    Error *err_handler; 

public: 
    ArffScanner(const string& _file);
    ~ArffScanner(); 

    bool nextLine();
    std::string nextWord();
    int lineNum; 
    bool isEOF;
    string curLine; 
    istringstream iss; 
};

#endif // ARFFSCANNER_H
