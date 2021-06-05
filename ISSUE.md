Todo and Issues
================

Todo
----------------

**Todo-001 [<u>Core/Stl/vector.h</u>]** Still have some functions to implement. (e.g. as earse() remove() find() ...).  

**Todo-002 [<u>Framework</u>]** Considering not to have EntryPoint.h embedded inside Common.  

**Todo-003 [<u>Common/Core/Defs.h</u>]** Seperate compile identification macros to other files. (e.g. __cplusplus__ version relative to Compilers.h)

**Todo-004 [<u>Core/FileSystem/FileSystem.cpp</u>]** Move all the set root directory operation to SEnv or something control all the environment variable.


Issues
----------------

**Issue-001 [<u>Framework</u>]** Still have so many platform relative problems.  Auto macro platform choose frontend is needed. (e.g. SG\_PLATFORM\_XXX)  

Considering
----------------

**Consider-001 [<u>Framework</u>]** Maybe we don't need allocator. Just have the memory allocation strategy is ok.  
