# Tau IR

An intermediate representation language intended for use with high level scripting, node based scripting, and string templating. The design goal is for the IR to be packaged into an EXE. The entry point will be set to an entry in the import table, this will cause an external DLL to be invoked at launch allowing for JIT'ing to happen, this is exactly how .NET EXE's work up to Windows XP (on later versions of Windows the OS will automatically recognize the headers and use an optimized loading procedure). The stored IR is a stack based architecture that is converted to an SSA architecture for optimization and JIT'ing.

## Concepts

The concepts folder contains various design documents or test code. The primary points of interest are the documentation of the instruction sets and the documents showing examples of the Tau IR to SSA conversion and optimization.

