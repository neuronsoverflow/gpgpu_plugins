#include "pluginEngine.h"
#include <iostream>
#include "helpers.h"
#include <boost/algorithm/string.hpp> // trim()

/******************************************************************************
*
******************************************************************************/
PluginEngine::PluginEngine()
{

}


/******************************************************************************
*
******************************************************************************/
void PluginEngine::run()
{
   std::string command;
   std::vector<std::string> cmdArgs;
   Plugin* plugin = NULL;
   std::map<std::string, Plugin*>::iterator it;

   while (command != "exit")
   {
      command = getCommand();
      cmdArgs = split(command, ' ');
      if (cmdArgs.empty())
         continue;

      if (!isValidCommand(cmdArgs))
      {
         std::cout << "Invalid command or invalid number of arguments\n";
         continue;
      }

      if (cmdArgs[0] == "load")
      {
         it = plugins.find(cmdArgs[1]);
         if (it == plugins.end())
         {
            loadPlugin(cmdArgs[1]);
         }
         else
         {
            std::cout << "The plugin is already loaded\n";
         }
      }
      else if (cmdArgs[0] == "view")
      {

      }
      else if (cmdArgs[0] == "set")
      {
      }
      else if (cmdArgs[0] == "run")
      {
         it = plugins.find(cmdArgs[1]);
         if (it == plugins.end())
         {
            // TODO: handle the error
            std::cout << "plugin not found!\n";
         }
         else
         {
            (it->second)->run();
         }

      }
      else if (cmdArgs[0] == "list")
      {

      }
      else if (cmdArgs[0] == "?")
      {
         displayOptions();
      }

   }

}

/******************************************************************************
*
******************************************************************************/
bool PluginEngine::isValidCommand(const std::vector<std::string>& cmd)
{
   int s = cmd.size();
   return (s > 0 &&
           ((cmd[0] == "load" && s == 2) ||
            (cmd[0] == "view" && s == 2) ||
            (cmd[0] == "set"  && s == 4) ||
            (cmd[0] == "run"  && s == 2) ||
            (cmd[0] == "list" && s == 1) ||
            (cmd[0] == "?"    && s == 1) ||
            (cmd[0] == "exit" && s == 1)));
}

/******************************************************************************
*
******************************************************************************/
void PluginEngine::display()
{

}

/******************************************************************************
*
******************************************************************************/
void PluginEngine::displayOptions()
{
   std::cout << "load <plugin_path>" << std::endl
             << "view <plugin_name>" << std::endl
             << "set <plugin_name> <key> <value>" << std::endl
             << "run <plugin_name>" << std::endl
             << "list - lists the loaded plugins and their parameters\n";
}

/******************************************************************************
*
******************************************************************************/
int PluginEngine::parseCommand(std::string cmd)
{
   return 0;
}

/******************************************************************************
*
******************************************************************************/
std::string PluginEngine::getCommand()
{
   std::string line;
   std::cout << ">";
   std::getline (std::cin, line);
   boost::trim(line); // get rid of leading and trailing whitespaces
   return line;
}

/******************************************************************************
*
******************************************************************************/
void PluginEngine::loadPlugin(std::string filename)
{
   std::map<std::string, Plugin*>::iterator it;
   it = plugins.find(filename);
   if (it != plugins.end())
   {
      // plugin is already inserted!
      // TODO: handle the error
      return;
   }

   try
   {
      std::cout << "loading: '" << filename << "'\n";
      plugins[shortName(filename)] = new Plugin(filename);
   }
   catch (char* e)
   {
      std::cout << e << std::endl;
   }
}
