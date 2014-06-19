// this class handles all the user interaction with the plugins
// the user can:
//   - load plugins
//   - view it's settings
//   - change it's settings / parameters
//   - run the program of a plugin, and etc...


#ifndef PLUGIN_ENGINE_H
#define PLUGIN_ENGINE_H

#include "plugin.h"
#include <map>
#include <string>
#include <vector>

class PluginEngine
{
   public:
      PluginEngine();       // check for GPU support
      void run();

   private:
      std::map<std::string, Plugin*> plugins;

      void display();
      void displayOptions();
      int parseCommand(std::string cmd); // set, run, get, ??????????????
      std::string getCommand();
      void loadPlugin(std::string filename);
      bool isValidCommand(const std::vector<std::string>& cmd);
};

#endif
