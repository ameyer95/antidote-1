#ifndef STRING_COMMON_H
#define STRING_COMMON_H

/**
 * Common string operations potentially used across multiple files
 */

#include <string>
#include <vector>

void removeWhitespace(std::string &s);
bool allWhitespace(const std::string &s);
void commaSplit(const std::string &s, std::vector<std::string> &store, bool trimWhitespace = false);
std::string toLowerCase(std::string s);

#endif
