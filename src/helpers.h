#ifndef HELPERS_H
#define HELPERS_H

#include <vector>
#include <string>

std::vector<std::string> &split(const std::string &s, char delim,
                                std::vector<std::string> &elems);

std::vector<std::string> split(const std::string &s, char delim);

std::string shortName(const std::string& filename);

#endif
