#ifndef ERROR_H
#define ERROR_H

#include <iostream> 

class Error {
private: 
    bool _isFatal; 
    bool _isWarning; 

public: 
    Error(); 
    void fatal(const std::string msg); 
    void warning(const std::string msg); 
    bool isFatal(); 
    bool isWarning(); 
    void resetFatal(); 
    void resetWarning(); 
};

#endif // ERROR_H