#ifndef PLUGINHEADER_H
#define PLUGINHEADER_H

#ifdef __cplusplus
#include <ctime> // clock_t
#else
#include <time.h> // clock_t
#endif

/*                             FUNCTION PROTOTYPES                           */
// we need the extern "C" when compiled w/ C++ so that the function
// symbol names won't get mangled for linking (useful for dlopen())
// see http://stackoverflow.com/questions/1041866/ for extern "C" info
#ifdef __cplusplus
extern "C"
{
#endif
    int run();

    int setParams(const char* buffer);
    int getParams(char* buffer, int bufferSize);

    // "getters"
    void* displayPluginInfo();
    const char* getParamInfo();
    const int getNumArgs();
    clock_t getRunTime();

#ifdef __cplusplus
}
#endif

#endif
