## A simple project template for testing libraries in C using the LLVM toolset, Make and valgrind

* Have the LLVM toolchain installed eg __clangd__ for IDE extensions, __clang-format__ for formatting the document, __lldb-vscode__ for __spacemacs__ and __lldb__ as well as __clang-format__ extensions in __vscode__

## Explanation by structure

1) The pipeline

* There is a simple __CI/CD__ pipeline that initially sets a __DEPLOYMENT__ flag to development so we can get a comprehensive list of warnings in the pipeline
* We also use __Valgrind__ for memory leaks or issues, as well as __check__ for unit testing
* We need to export the __LD_LIBRARY_PATH__ in **BOTH** the local (your pc) and remote (the pipeline) enviroments (I am currently doing that manually per working directory)
* Then we build with some production flags and finally deliver the library

2) .vscode/

* We need this folder in order to tell lldb-vscode how to act when debugging
* We use the __./vscode/jaunch.json__ to hook a launch process by invoking the __test__ recipe (which builds the __test__ executable to run the tests) from the __Makefile__ (more on that later)
* We use the __./vscode/tasks.json__ to define the __build__ task which builds a __test executable__ with **development flags**

3) The src directory

* Each file shall have its __correposnding header__ and a __vice versa__ (I believe this logic gives a modern approach like C# or Java ehile it doesnt increase the actual complexity)
* Go wild, the __build__ recipe will take care of the whole building process (more on that later)

4) the tests directory

* Declare a __START_TEST__ with a corresponding name within the arguments
* Add the test case in the suite function by name

5) The weird Makefile

* Although the __makefile__ is structured in a weird way eg traversing all the files upon each build, which makes it not suitable for big projects, it does remind of an abstration and thus making it easier to extend it
* It can distinguish the __production__ and __development__ env variables
* __memory check__ with build the library, tests, run them and also generate a __Valgrind__ report  

6) The LLVM part

* There are a few solid configurations in order to boost your C++ knowledge by adding a lot of warnings and standards
* Nice to use a formatter for the whole project

Feel free to open a PR if you want to extend the template!
