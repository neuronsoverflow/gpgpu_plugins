#include "plugin.h"
#include "constants.h"
#include "helpers.h"
#include <cstring> // strcmp
#include <dlfcn.h> // dlopen(), dlsym(), dlclose(), dlerror() - requires: -ldl
#include <iostream> // cout
#include <vector>

/******************************************************************************
*
******************************************************************************/
Plugin::Plugin(const std::string& filename) throw(char*)
{
   this->filename = filename;

   // TODO: make a couple of checks before doing the following...
   // name.insert(0, "./");
   // name.append(".so");

   // TODO: start clock, and time how long took to load plugin [requirements]

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

   // TODO: once is all loaded, remove the absolute path from the plugin name
   //or save a new variable called shortName w/ the path and extension stripped
   // SET NAME here

}

/******************************************************************************
*
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
*
******************************************************************************/
void Plugin::displayParams() const
{
                            // PARAM_NAME:  PARAM_VALUE
                            // input1: ""
                            // output: out
   std::map<std::string, std::string>::const_iterator it;
   std::cout << "PARAM NAME:\t\tPARAM_VALUE\n";
   for (it = params.begin(); it != params.end(); ++it)
      std::cout << it->first << " => " << it->second << '\n';
}

/******************************************************************************
* // getParams() that sets the params map
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
      // TODO: handle it the error
   }

   std::vector<std::string> keys = split(bufferKeys, ',');
   std::vector<std::string> vals = split(bufferValues, DELIM);

   if (keys.size() != vals.size())
   {
      // TODO: handle the error
      // printf("Unexpected error!!!\n");
   }

   for (unsigned int i = 0; i < keys.size(); i++)
      params[keys[i]] = vals[i];
}

/******************************************************************************
* updateParams()
* - reads the parameters from the `params` map, builds a string with all
*   parameters and sets those values in the plugin w/ setParams()
* // setParams() from the values in the params map
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
      // TODO: handle the error
   }
}

/******************************************************************************
*
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
      // TODO: handle the error
   }
}
