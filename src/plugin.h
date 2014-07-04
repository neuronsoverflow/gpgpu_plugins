#ifndef PLUGIN_H
#define PLUGIN_H

#include <string>
#include <ctime> // clock_t
#include <map>

// setup the function pointer types
typedef int         (*setP_f)  (const char*);
typedef int         (*getP_f)  (char*, int);
typedef int         (*run_f)   ();
typedef const char* (*query_f) ();
typedef clock_t     (*time_f)  ();

// a Wrapper to the loading the dynamic library w/ dlopen...
class Plugin
{
   public:
      Plugin(const std::string& filename) throw(char*);
      ~Plugin();

      int run();
      const char* getInfo(); // returns a string literal
      int getParams(char* buffer, int bufferSize);
      int setParams(const char* buffer);
      clock_t getRunTime();

      void setParam(std::string key, std::string value);

      std::string getName() const { return name; }
      int getSize() const         { return numParams; }

      void displayParams() const;

   private:
      void* handle; // plugin handler

      // plugin function handlers
      run_f   fRun;
      query_f fQuery;
      getP_f  fGetParams;
      setP_f  fSetParams;
      time_f  fTime;
      int numParams; // the number of parameters this plugin has

      std::string name;     // plugin name

      std::map<std::string, std::string> params; // key: pName, val: pValue

      void loadSymbols() throw(char*);
      void refreshParams(); // getParams() that sets the params map
      void updateParams();  // setParams() from the values in the params map
};

// this class also needs to keep track of parameters, and make it easy to change just a single parameter (instead of all ... )
/*
setParam(string key, string value);
 */

#endif
