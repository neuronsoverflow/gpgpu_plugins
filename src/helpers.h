#ifndef HELPERS_H
#define HELPERS_H

#include <string>
#include <vector>

// returns a vector containing the split string
std::vector<std::string> split(const std::string &s, char delim);

// returns the file name without the file path and without the extension
std::string shortName(const std::string& filename);

// trims the given string
void trim(std::string& str);

#endif
