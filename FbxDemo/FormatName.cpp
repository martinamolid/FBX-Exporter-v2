#include "FormatName.h"

/*
========================================================================================================================

	This function returns wrong the way it is right now, does not work.

	// Martina Molid

========================================================================================================================
*/

char* FormatName(std::string tempName) {

	int nameLength = strlen(tempName.c_str());
	char name[NAME_SIZE];
	for (int i = 0; i < nameLength; i++) {
		name[i] = tempName[i];
	}
	name[nameLength] = '\0'; // Puts a \0 at the end of the name, still printing out whitespace into the binary file
	return name;
}