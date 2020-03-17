#ifndef ARFFSCANNER_H
#define ARFFSCANNER_H

#include <string> 
#include <iostream> 
#include <fstream>
#include <sstream>

using namespace std;

class ArffScanner {
public: 
    ArffScanner(const string& _file);

    ~ArffScanner(); 

    bool nextLine();

    std::string nextWord();
    
    bool isEOF;

    string curLine; 
    
    istringstream iss; 
private:
    string file;
    ifstream fp;
};

#endif // ARFFSCANNER_H
