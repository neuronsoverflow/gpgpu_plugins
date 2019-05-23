#ifndef PLUGIN_ENGINE_H
#define PLUGIN_ENGINE_H

#include "plugin.h"
#include <map>
#include <string>
#include <vector>

/******************************************************************************
 * PluginEngine class
 *  this class handles all the user interaction with the plugins.
 *  the user is able to:
 *  - load plugins
 *  - view the plugin's parameters
 *  - set the plugin's parameters
 *  - run the program of a plugin
 *  - list all loaded plugins and their parameters
 ******************************************************************************/
class PluginEngine
{
  public:
    PluginEngine();
    void run();

  private:
    std::map<std::string, Plugin*> plugins;

    void displayPlugins(); // lists all loaded plugins and their parameters
    void displayOptions();
    void displayRunTime(const std::map<std::string, Plugin*>::iterator& it);
    std::string getCommand();
    void loadPlugin(std::string filename);
    bool isValidCommand(const std::vector<std::string>& cmd);
};

#endif
