#include "plugin.h"
#include "constants.h"
#include "helpers.h"
#include <cstring> // strcmp
#include <dlfcn.h> // dlopen(), dlsym(), dlclose(), dlerror() - requires: -ldl
#include <iostream> // cout
#include <vector>

/******************************************************************************
* Plugin handler class constructor
******************************************************************************/
Plugin::Plugin(const std::string& filename) throw(char*)
{
   // attempt to load the plugin handler
   handle = dlopen(filename.c_str(), RTLD_NOW);

   if (handle == NULL)
      throw (dlerror());

   // now load the function symbols
   try
   {
      loadSymbols();
   }
   catch(char* e)
   {
      throw(e);
   }

   refreshParams(); // load the parameters from the plugin

   this->name = shortName(filename);
}

/******************************************************************************
* Plugin destructor
******************************************************************************/
Plugin::~Plugin()
{
   dlclose(handle);
}

/******************************************************************************
* /////////////////////////////   wrapper functions   /////////////////////////
******************************************************************************/
int Plugin::run()
{
   updateParams(); // make sure to set the plugin parameters before running
   return fRun();
}

char* Plugin::getInfo()
{
   return fQuery();
}

int Plugin::getParams(char* buffer, int bufferSize)
{
   return fGetParams(buffer, bufferSize);
}

int Plugin::setParams(const char* buffer)
{
   return fSetParams(buffer);
}

clock_t Plugin::getRunTime()
{
   return fTime();
}

/******************************************************************************
* loadSymbols
* - assigns a pointer to each function that will be used in the plugin
* - see: man 3 dlsym
******************************************************************************/
void Plugin::loadSymbols() throw(char*)
{
   char* error;

   // load the function pointers
   dlerror();    /* Clear any existing error */
   fSetParams = reinterpret_cast<setP_f>(dlsym(handle, "setParams"));
   if ((error = dlerror()) != NULL)
      throw(error);

   dlerror();    /* Clear any existing error */
   fGetParams = reinterpret_cast<getP_f>(dlsym(handle, "getParams"));
   if ((error = dlerror()) != NULL)
      throw(error);

   dlerror();    /* Clear any existing error */
   fRun = reinterpret_cast<run_f>(dlsym(handle, "run"));
   if ((error = dlerror()) != NULL)
      throw(error);

   dlerror();    /* Clear any existing error */
   fQuery = reinterpret_cast<query_f>(dlsym(handle, "queryParamInfo"));
   if ((error = dlerror()) != NULL)
      throw(error);

   dlerror();    /* Clear any existing error */
   fTime = reinterpret_cast<time_f>(dlsym(handle, "getRunTime"));
   if ((error = dlerror()) != NULL)
      throw(error);

   // load numParams = NUM_ARGS pointer
   dlerror();    /* Clear any existing error */
   int* pNumParams = reinterpret_cast<int*>(dlsym(handle, "NUM_ARGS"));
   if ((error = dlerror()) != NULL)
      throw(error);
   this->numParams = *pNumParams;
}

/******************************************************************************
* displayParams()
* - displays the plugin's parameter keys and values
******************************************************************************/
void Plugin::displayParams() const
{
   std::cout << name << ":\n";
   if (params.size())
   {
      std::map<std::string, std::string>::const_iterator it;
      std::cout << "\tPARAM NAME => PARAM_VALUE\n";
      for (it = params.begin(); it != params.end(); ++it)
         std::cout << "\t" << it->first << " => " << it->second << '\n';
   }
   else
      std::cout << "\tThis plugin has no parameters\n";
}

/******************************************************************************
* refreshParams()
* - reads the parameters from the plugin and stores them in the params map
******************************************************************************/
void Plugin::refreshParams()
{
   if (!numParams)
      return;

   // read the parameters from the plugin, and insert them in the map
   int error;
   char bufferKeys[1024];
   char bufferValues[1024];
   char* info = getInfo();
   strcpy(bufferKeys, info);
   error = getParams(bufferValues, 1024);
   if (error == ERROR)
   {
      std::cout << "Unexpected error in " << __PRETTY_FUNCTION__ << std::endl;
   }

   std::vector<std::string> keys = split(bufferKeys, ',');
   std::vector<std::string> vals = split(bufferValues, DELIM);

   if (keys.size() != vals.size())
   {
      std::cout << "Unexpected error in " << __PRETTY_FUNCTION__ << std::endl;
   }

   for (unsigned int i = 0; i < keys.size(); i++)
      params[keys[i]] = vals[i];
}

/******************************************************************************
* updateParams()
* - reads the parameters from the `params` map, builds a string with all
*   parameters and sets those values in the plugin w/ setParams()
******************************************************************************/
void Plugin::updateParams()
{
   std::string values;
   std::map<std::string, std::string>::const_iterator it;
   for (it = params.begin(); it != params.end(); ++it)
      values += it->second + DELIM;
   if (values.size())
      values[values.size() - 1] = '\0'; // delete the last inserted DELIM
   if (setParams(values.c_str()) == ERROR)
   {
      std::cout << "Unexpected error in " << __PRETTY_FUNCTION__ << std::endl;
   }
}

/******************************************************************************
* setParam()
* - changes the value of the parameter with that has the given key
******************************************************************************/
void Plugin::setParam(std::string key, std::string value)
{
   std::map<std::string, std::string>::iterator it;

   it = params.find(key);
   if (it != params.end())
   {
      it->second = value;
   }
   else
   {
      std::cout << "invalid parameter key\n";
   }
}
