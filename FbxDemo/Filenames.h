#pragma once

/*
========================================================================================================================

	Filenames.h holds all filenames to be used for the run of the program.
	**This should be included in any file intending to use the filenames.

	ASCII_FILE is the filename for the textfile to which the ASCII version of the object will be printed to,
		this is the "cheat sheet" for what is in the binary file, but structured in a easily read way.
	BINARY_FILE is the filename for the binary file which is the main file we write to, and which will be read from,
		this will be used the most, as the file has to be opened and closed in different functions.
	IN_FBX_FILEPATH is the .fbx file which will be converted into our custom binary format.

	NAME_SIZE is used for printing names a specific size to be read in.

	// Martina Molid

========================================================================================================================
*/

#include <string>

#define NAME_SIZE 256

const std::string ASCII_FILE = "Exported_Files/xTestAscii.txt";
const std::string BINARY_FILE = "Exported_Files/Level1[Plain].bin"; //Target file.
const std::string IN_FBX_FILEPATH = "FBX_Files/Level1[Plain].fbx"; //Source file.

// MM: Standard names, in case you mess them up too much
//const std::string ASCII_FILE = "Exported_Files/xTestAscii.txt";
//const std::string BINARY_FILE = "Exported_Files/xTestBinary.bin";
//const std::string IN_FBX_FILEPATH = "FBX_Files/cube.fbx";