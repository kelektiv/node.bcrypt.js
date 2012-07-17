@echo off
@rem Check for nodejs build location variable
if not defined NODE_ROOT goto nodebuild-not-found
if not exist "%NODE_ROOT%\src\node.h" goto nodebuild-not-found
if not exist "%NODE_ROOT%\deps\v8\include\v8.h" goto nodebuild-not-found
if not exist "%NODE_ROOT%\deps\uv\include\uv.h" goto nodebuild-not-found
if not exist "%NODE_ROOT%\deps\openssl\openssl\include\openssl\opensslv.h" goto nodebuild-not-found
if not exist "%NODE_ROOT%\tools\gyp\gyp" goto gyp-not-found

@rem detect the location of the node.lib file
set node_lib_folder=
if exist "%NODE_ROOT%\Release\node.lib" set node_lib_folder=Release
if not defined node_lib_folder if exist "%NODE_ROOT%\Debug\node.lib" set node_lib_folder=Debug
if not defined node_lib_folder goto nodebuild-not-found

@rem Check if user has specified the build command
set requestedBuild= 0
@rem Check if the first argument is make
if "%1" == "make" set requestedBuild= 1
if %requestedBuild% == 0 echo Run "node-gyp make" if you want to generate the project and build it at once.
@rem Try to locate the gyp file
set gypfile=
if exist "module.gyp" set gypfile=module.gyp
if not defined gypfile goto gyp-file-missing
@rem Generate visual studio solution
python %NODE_ROOT%\tools\gyp\gyp -f msvs -G msvs_version=2010 %gypfile% --depth=. -DNODE_ROOT=%NODE_ROOT% -Dnode_lib_folder=%node_lib_folder% -DICU_root=%ICU_ROOT%
if errorlevel 1 goto exit-error
if %requestedBuild% == 0 goto Finished
@rem Build the solution
@rem Check for visual studio tools if not already loaded
if defined VCINSTALLDIR goto BuildSolution
@rem Ensure that visual studio is available
if not defined VS100COMNTOOLS goto msbuild-not-found
if not exist "%VS100COMNTOOLS%..\..\vc\vcvarsall.bat" goto msbuild-not-found
call "%VS100COMNTOOLS%..\..\vc\vcvarsall.bat"
@rem Check that vs is properly loaded
if not defined VCINSTALLDIR goto msbuild-not-found
:BuildSolution
call :BuildSolutionBasedOnGypFilename %gypfile%
:Finished
echo Finished
goto exit
:msbuild-not-found
echo Visual studio tools were not found! Please check the VS100COMNTOOLS path variable
goto exit
:gyp-not-found
echo GYP was not found. Please check that gyp is located in %NODE_ROOT%/tools/gyp/ 
goto exit
:nodebuild-not-found
echo Node build path not found! Please check the NODE_ROOT environment variable exists and that it points to the root of the git repo where you have build  node
goto exit
:gyp-file-missing
echo Could not locate a gyp file. module.gyp file was not found.
goto exit
:msbuild-not-found
echo Visual studio tools were not found! Please check the VS100COMNTOOLS path variable
goto exit
:solutions-file-not-found
echo Generated solution file %generatedslnfile% was not found!
goto exit
:exit-error
echo An error occured. Please check the above output
:exit
@rem clear local variables
set node_lib_folder=
set requestedBuild=
set gypfile=
goto :EOF
@rem Internal procedures
:BuildSolutionBasedOnGypFilename
set generatedslnfile=%~n1.sln
if not exist %generatedslnfile% goto solutions-file-not-found
@rem call msbuild to build the sln file
msbuild.exe %generatedslnfile%
if errorlevel 1 goto exit-error
goto :EOF
