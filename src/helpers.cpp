#include "helpers.h"
#include <sstream>

/******************************************************************************
* http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
******************************************************************************/
std::vector<std::string> &split(const std::string &s, char delim,
                                std::vector<std::string> &elems)
{
   std::stringstream ss(s);
   std::string item;
   while (std::getline(ss, item, delim))
      elems.push_back(item);

   return elems;
}

/******************************************************************************
* split()
******************************************************************************/
std::vector<std::string> split(const std::string &s, char delim)
{
   std::vector<std::string> elems;
   split(s, delim, elems);
   return elems;
}

/******************************************************************************
* shortName()
* - returns the short version of the given filename
* - example: "./plugins/hello.so" => "hello"
******************************************************************************/
std::string shortName(const std::string& filename)
{
   std::string result = filename;
   int pos = result.find_last_of("/") + 1; // strip leading directories
   result = result.substr(pos);
   std::string extension = ".so"; // strip .so extension (if any)
   pos = result.find(extension);
   if (pos != std::string::npos)
      result = result.substr(0, pos);
   extension = ".dll"; // strip .dll extension (if any)
   pos = result.find(extension);
   if (pos != std::string::npos)
      result = result.substr(0, pos);
   return result;
}
