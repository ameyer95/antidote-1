#include "Error.h"

Error::Error() {
    _isFatal = false; 
    _isWarning = false; 
}

void Error::fatal(const std::string msg) {
    std::cout << "Error: " << msg << std::endl; 
    _isFatal = true;
}

void Error::warning(const std::string msg) {
    std::cout << "Warning: " << msg << std::endl; 
    _isWarning = true;
}

bool Error::isFatal() {
    return _isFatal; 
}

bool Error::isWarning() {
    return _isWarning; 
}

void Error::resetFatal() {
    _isFatal = false;  
}

void Error::resetWarning() {
    _isWarning = false; 
}
