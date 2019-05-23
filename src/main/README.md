./gpgpu
=========

Description
----

All the files in this folder are intended to build the plugin engine, `gpgpu`

This program is able to load dynamic shared libraries that conform to the plugin format.

Usage
----
The following are the program's options:
```
Options:
	load <plugin_path> - loads a plugin into the program
	view <plugin_name> - displays the plugin's parameters
	set  <plugin_name> <key> <value> - set a parameter for the plugin
	run  <plugin_name> - executes the plugin's main program
	time <plugin_name> - displays the time for the last run()
	help <plugin_name> - shows the plugin's help instructions
	list - lists the loaded plugins and their respective parameters
	? - show these options
	exit
```

Example
-----------
A sample run could look like this
```sh
make # creates ./gpgpu and the plugins/*.so plugins
./gpgpu
# the options are displayed...
load plugins/hello.so # load a plugin
run hello # run the main program of that plugin
Hello World! # this is the result of the execution

# prime numbers example
load plugins/prime.so
set prime limit 1000 # set the upper limit. this will make it check numbers 2-1000 for primality
set prime outputFile primes_result.txt
run primes

# after the primes is run, the result will be in the file
exit

cat primes_result.txt
```

