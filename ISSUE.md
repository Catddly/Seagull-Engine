Todo and Issues
================

Todo
----------------

**Todo-001 [<u>Common/Core/Defs.h</u>]** Seperate compile identification macros to other files. (e.g. __cplusplus__ version relative to Compilers.h)

**Todo-002 [<u>Core/FileSystem/FileSystem.cpp</u>]** Move all the set root directory operation to SEnv or something control all the environment variable.

**Todo-002 [<u>Common/System/ISystem.h</u>]** Abstract the modules to core modules and modules, beacause my code need the OCP(Open Closed Principle) and LSP(Liskov Substitution Principle).

Issues
----------------

**Issue-001 [<u>Framework</u>]** Still have so many platform relative problems.  Auto macro platform choose frontend is needed. (e.g. SG\_PLATFORM\_XXX)  

Considering
----------------

**Consider-001 [<u>Framework</u>]** Maybe we don't need allocator. Just have the memory allocation strategy is ok.  


**Consider-001 [<u>Framework</u>]** Currently, i don't be satisfied with the framework of my engine, we want to have more judgement and design.  

