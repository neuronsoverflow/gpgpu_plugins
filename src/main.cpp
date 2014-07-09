#include <iostream>
#include <dlfcn.h> // dlopen(), dlsym(), dlclose(), dlerror() - requires: -ldl
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include <cstdio> // printf

#include "pluginEngine.h"

using namespace std;
int main(int argc, char** argv)
{
   try
   {
      PluginEngine engine;
      engine.run();
   }
   catch(char* e)
   {
      cout << e << endl;
   }


   return 0;
}
