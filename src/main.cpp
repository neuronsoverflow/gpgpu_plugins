// right now, just write a simple function to query for the device status
#include <iostream>
#include <dlfcn.h> // dlopen(), dlsym(), dlclose(), dlerror() - requires: -ldl
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include <cstdio> // printf
#include "plugin.h"


/*
main class:
loop, displying the options,
- load plugin
- execute command on loaded plugin
- commands:
   - run()
   - getInfo()
   - getParams()
   - setParams()
   - getRunTime()


 */


#include "pluginEngine.h"

using namespace std;
int main(int argc, char** argv)
{
   string pluginName;
   // void* plugin;
   // char* error;


   // cout << "Load what plugin?\n>";
   // cin >> pluginName;

   try
   {
      PluginEngine engine;
      engine.run();

      // Plugin p(pluginName);
      // cout << "displaying the params\n"; p.displayParams();
      // cout << "refreshing the params\n"; p.refreshParams();
      // cout << "displaying the params\n"; p.displayParams();
      // cout << "updating the params\n"; p.updateParams();
      // cout << "refreshing the params\n"; p.refreshParams();
      // cout << "updating the params\n"; p.updateParams();
      // cout << "refreshing the params\n"; p.refreshParams();
      // cout << "updating the params\n"; p.updateParams();
      // cout << "refreshing the params\n"; p.refreshParams();
      // cout << "updating the params\n"; p.updateParams();
      // cout << "refreshing the params\n"; p.refreshParams();
      // cout << "displaying the params\n"; p.displayParams();

      return 0;
      // p.run();

      // cout << "plugin wants:" << p.getInfo() << endl;


   }
   catch(char* e)
   {
      cout << e << endl;
   }


   return 0;
}
