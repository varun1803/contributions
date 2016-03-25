# Compiler for Decaf language
Compiler to produce llvm IR for a given code in decaf language with specifications as explained in the link below:

[**Decaf Syntax Specifications**](http://compilers.iiit.ac.in/mediawiki/images/c/c2/Decafspec.pdf)

## Prerequisites
> Bison and Flex installed to work with C++

> llvm compiler installed

## Usage
> `./syntax_analyser <test_file_name> <optional/required only to retain the intermediate files>`

> After running the code and saving the output in a file, this script deletes the files created while compilation. To retain those files pass the 2nd parameter to the syntax_analyser script as **1**

> Test files are in the folder **test_cases**
## Files
> Bison file: bison.y

> Flex file: syntax_analyser.l

> Ast header file: ast.h

> Codegen files: codegen.h codegen.cpp


## Output
> Dump of the IR generation in stored in **IR_dump.txt**. To check the output run `lli IR_dump.txt`

> Output of the codegen is stored in **IR.txt**

> To remove the output files and other unnecessary files in the folder run: `make clean`

> To produce the debug files run: `make debug`

## Features
> This code has been tested on ubuntu. If this does not works with any other linux distro, try to check the installation process of bison, flex and llvm for that particular distro. Also Make file might not work with all the linux distros, so you need to change the linking part in the compilation process yourself.

> Can parse complete decaf syntax and create ast for any and all valid syntax of decaf language.

> Can create llvm IR for following codes containing any and all of the following cases together:
> + **All binary and arithmetic operations**, 
> + **IF_ELSE** (nested as well), 
> + **FOR_Loops**, 
> + **Global_Variables (declaration, assignemnt and usage anywhere in the program)**,
> + **Function_Calls (recursive as well)**, 
> + **Callout** 