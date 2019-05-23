#include "pluginEngine.h"
#include "helpers.h"
#include <ctime>
#include <iostream>

using namespace std;

/******************************************************************************
 * PluginEngine constructor
 ******************************************************************************/
PluginEngine::PluginEngine()
{
    // NOTE: was planning on checking for GPU support here, but that's the
    // plugin's task now... (see checkDevice() in the .cu files)
}

/******************************************************************************
 * run()
 * - displays the options
 * - continually prompt the user for commands and execute them
 * - stops when the user uses "exit"
 ******************************************************************************/
void PluginEngine::run()
{
    string command;
    vector<string> cmdArgs;
    map<string, Plugin*>::iterator it;

    displayOptions();

    while (command != "exit")
    {
        command = getCommand();        // prompt the user
        cmdArgs = split(command, ' '); // put the args in a vector

        if (cmdArgs.empty())
            continue;

        cout << '\n';

        if (!isValidCommand(cmdArgs))
        {
            cout << "Invalid command or invalid number of arguments\n";
            continue;
        }

        // make the iterator point to the plugin (this is used in view, set, run)
        it = plugins.find(cmdArgs[1]);
        if (it == plugins.end() && (cmdArgs[0] == "view" || cmdArgs[0] == "set" || cmdArgs[0] == "run" ||
                                    cmdArgs[0] == "time" || cmdArgs[0] == "help"))
        {
            cout << "plugin not found!\n";
            continue;
        }

        // go through each command option and execute the appropriate actions
        if (cmdArgs[0] == "load") // load <plugin_path>
        {
            loadPlugin(cmdArgs[1]);
        }
        else if (cmdArgs[0] == "view") // view <plugin_name>
        {
            (it->second)->displayParams();
        }
        else if (cmdArgs[0] == "set") // set <plugin_name> <key> <value>
        {
            (it->second)->setParam(cmdArgs[2], cmdArgs[3]);
        }
        else if (cmdArgs[0] == "run") // run <plugin_name>
        {
            (it->second)->run();
            displayRunTime(it);
        }
        else if (cmdArgs[0] == "time") // time <plugin_name>
        {
            displayRunTime(it);
        }
        else if (cmdArgs[0] == "help") // help <plugin_name>
        {
            (it->second)->displayPluginInfo();
        }
        else if (cmdArgs[0] == "list")
        {
            displayPlugins();
        }
        else if (cmdArgs[0] == "?")
        {
            displayOptions();
        }
    }
}

/******************************************************************************
 * getCommand()
 * - prompts the user for the command
 ******************************************************************************/
string PluginEngine::getCommand()
{
    string line;
    cout << ">";
    getline(cin, line);
    trim(line); // get rid of leading and trailing whitespaces
    return line;
}

/******************************************************************************
 * usValidCommand()
 * - returns true if the the vector of commands is valid
 ******************************************************************************/
bool PluginEngine::isValidCommand(const vector<string>& cmd)
{
    int s = cmd.size();
    return (s > 0 && ((cmd[0] == "load" && s == 2) || // load <plugin_path>
                      (cmd[0] == "view" && s == 2) || // view <plugin_name>
                      (cmd[0] == "set" && s == 4) ||  // set  <plugin_name> <key> <value>
                      (cmd[0] == "run" && s == 2) ||  // run  <plugin_name>
                      (cmd[0] == "time" && s == 2) || // time  <plugin_name>
                      (cmd[0] == "help" && s == 2) || // help <plugin_name>
                      (cmd[0] == "list" && s == 1) || // list
                      (cmd[0] == "?" && s == 1) ||    // ?
                      (cmd[0] == "exit" && s == 1))); // exit
}

/******************************************************************************
 * loadPlugin()
 * - loads the plugin into the program
 * - when a plugin is successfully loaded, the plugins map is set to contain it
 ******************************************************************************/
void PluginEngine::loadPlugin(string filename)
{
    string pluginName = shortName(filename);
    map<string, Plugin*>::iterator it;
    it = plugins.find(pluginName);
    if (it != plugins.end())
    {
        cout << "The plugin \"" << pluginName << "\" is already loaded\n";
        return;
    }

    try
    {
        clock_t start_t = clock();
        clock_t total_t;
        cout << "loading: '" << filename << "'\n";
        plugins[pluginName] = new Plugin(filename);
        total_t = clock() - start_t;
        cout << "plugin loaded successfully in " << (total_t * 1000) / (double)CLOCKS_PER_SEC << "ms\n"
             << "Displaying it's parameters:\n";
        plugins[pluginName]->displayParams();
    }
    catch (char* e)
    {
        cout << e << endl;
    }
}

/******************************************************************************
 * displayPlugins()
 * - displays the list of plugins
 * - displays the parameters of each plugin
 ******************************************************************************/
void PluginEngine::displayPlugins()
{
    if (!plugins.size())
    {
        cout << "No plugins are loaded\n";
        return;
    }

    // display the names of the plugins
    map<string, Plugin*>::iterator it;
    cout << "Loaded plugins:\n";
    for (it = plugins.begin(); it != plugins.end(); it++)
    {
        cout << "\t - " << it->first << endl;
    }

    // display the parameters of the plugins
    for (it = plugins.begin(); it != plugins.end(); it++)
    {
        (it->second)->displayParams();
    }
}

/******************************************************************************
 * displayRunTime() - shows how long the plugin's last run() command took
 ******************************************************************************/
void PluginEngine::displayRunTime(const map<string, Plugin*>::iterator& it)
{
    // prime plugin's last run took: 0.000 seconds
    cout << "\"" << it->first << "\" plugin's last run took: " << (it->second)->getRunTime() / (double)CLOCKS_PER_SEC
         << " seconds\n";
}

/******************************************************************************
 * displayOptions()
 * - displays the available options for the user
 ******************************************************************************/
void PluginEngine::displayOptions()
{
    cout << "Options:\n";
    cout << "\tload <plugin_path> - loads a plugin into the program\n"
         << "\tview <plugin_name> - displays the plugin's parameters\n"
         << "\tset  <plugin_name> <key> <value> - set a parameter for the plugin\n"
         << "\trun  <plugin_name> - executes the plugin's main program\n"
         << "\ttime <plugin_name> - displays the time for the last run()\n"
         << "\thelp <plugin_name> - shows the plugin's help instructions\n"
         << "\tlist - lists the loaded plugins and their respective parameters\n"
         << "\t? - show these options\n"
         << "\texit\n";
}
