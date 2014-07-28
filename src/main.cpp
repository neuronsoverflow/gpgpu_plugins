#include "pluginEngine.h"
#include <cstring> // strcmp
#include <iostream>

int main(int argc, char** argv)
{
   if (argc > 1 && (strcmp(argv[1], "--help") == 0))
   {
      std::cout << "This program loads plugins that can be run on the GPU.\n"
                << "When running the program, use the '?' command"
                << " to show it's usage\n  and options.\n";
      return 0;
   }
   try
   {
      PluginEngine engine;
      engine.run();
   }
   catch(char* e)
   {
      std::cout << e << std::endl;
   }


   return 0;
}
