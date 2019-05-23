#ifndef CONSTANTS_H
#define CONSTANTS_H

// these constants need to be compatible w/ C and C++

static const char US = 0x1F;           // Unit Separator (octal): "\037"; (hex): "\0x1f";
static const char DELIM = 0x1F;        // Use US for the Delimeter separator
static const char* DELIM_STR = "\037"; // US in octal
static const int BUFFER_SIZE = 256;    // size of buffers passed to/from functions
static const int ERROR = -1;
static const int OK = 0;

#endif
