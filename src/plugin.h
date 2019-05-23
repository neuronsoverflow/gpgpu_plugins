#ifndef PLUGIN_H
#define PLUGIN_H

#include "orderedMap.h"
#include <ctime> // clock_t
#include <string>

// setup the function pointer types
typedef int (*run_f)();               // run()
typedef int (*setP_f)(const char*);   // setParams()
typedef int (*getP_f)(char*, int);    // getParams()
typedef void* (*pluginInfo_f)();      // getPluginInfo()
typedef const char* (*paramInfo_f)(); // getParamInfo()
typedef int (*numArgs_f)();           // getNumArgs()
typedef clock_t (*time_f)();          // getRunTime()

/******************************************************************************
 * Plugin Class
 * This is a Wrapper class for loading the dynamic libraries w/ dlopen...
 * Note that the function pointers must match the functions found in:
 *   - plugins/include/pluginHeader.h
 ******************************************************************************/
class Plugin
{
  public:
    Plugin(const std::string& filename);
    ~Plugin();

    int run();
    int setParams(const char* buffer);
    int getParams(char* buffer, int bufferSize);
    void* displayPluginInfo();
    const char* getParamInfo();
    int getNumArgs();
    clock_t getRunTime();

    void setParam(std::string key, std::string value);

    std::string getName() const { return name; }
    int getSize() const { return numParams; }

    void displayParams() const;

  private:
    void* handle; // plugin handler

    // plugin function handlers
    run_f fRun;
    setP_f fSetParams;
    getP_f fGetParams;
    pluginInfo_f fPluginInfo;
    paramInfo_f fParamInfo;
    numArgs_f fNumArgs;
    time_f fTime;
    int numParams; // the number of parameters this plugin has

    std::string name; // plugin name

    OrderedMap params;

    void loadSymbols();
    void refreshParams(); // getParams() that sets the params map
    void updateParams();  // setParams() from the values in the params map
};

#endif
