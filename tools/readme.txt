Please define a system variable called NODE_ROOT, and point it to the node source folder, the one that contains the src and deps.

To build on windows:
You can then execute "node-gyp.bat" to generate the solution and proj files or execute "node-gyp.bat make" which will build the module. You need the Windows c++ runtime to run, 
and to build you need Visual Studio (express is fine).

To build on unix-based:
You can still use waf on unix to build or execute node-gyp to generate a makefile with gyp.