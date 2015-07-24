## Plugin Structure

### Conventions

Each plugin is kept in separate directory inside src/ folder written in lowercase convention.

### Structure

Each plugin contains following structure:
* ```<pluginname>.gyp```
* ```<pluginname>_api.js```
* ```<pluginname>_extension.h```
* ```<pluginname>_extension.cc```
* ```<pluginname>_instance.h```
* ```<pluginname>_instance.cc```

### Spec file

Spec file (```webapi-plugins.spec```) kept inside ```packaging/``` directory
is build specification file used by rpm packaging system where variables are defined.
Those variables can be used to include or exclude particular modules from
build for each profile (mobile, TV, wearable).

### GYP file

Each plugin has its own gyp file that contains information specific for it.
Plugin configuration file (gyp) is the equivalent of CMake.
It contains information what files to build what libraries to use for linking.
There can be also found one main gyp file in ```src/``` folder (```tizen-wrt.gyp```)
that includes others.

### Implementation files

Description of files required in plugin implementation.
* **C++ files** (```<pluginname>_extension.h, <pluginname>_extension.cc```)<br>
  Extension namespace and other objects exported by JavaScript layer are set inside these files.
* **C++ files** (```<pluginname>_instance.h, <pluginname>_instance.cc```)<br>
  These files are responsible for communication between JavaScript layer and Native API.
* **JavaScript file** (```<pluginname>_api.js```)<br>
  This file contains all methods required by each API.
  All operation should be done by JavaScript as much as possible.
  If JavaScript can do something, it should do it.
  This file is responsible for checking privileges, checking arguments, calling C++ methods etc.

### Plugin flow

![Plugin flow](images/plugin_flow.png)

Explanation of steps:
1. From JavaScript Layer information is sent to C++ Layer.
   This information consists of type of call (asynchronous, synchronous)
   arguments given by user, any additional information that is required to
   successfully acquire required data. Data is sent in form of JSON.
2. C++ parses acquired JSON. After the data is processed. Appropriate platform
   functions are called with the specified arguments.
3. Platform returns specified values to C++ layer.
4. Another JSON is formed. It consists of data that was acquired from platform.
