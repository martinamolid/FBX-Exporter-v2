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

/*=================================================================================
	Filehandling will be handled with the command prompt later
=================================================================================*/


#include <vector>
#include <string>

using namespace std;

#define NAME_SIZE 256

const std::string ASCII_FILE = "Exported_Files/xTestAscii.txt";
const std::string BINARY_FILE = "Exported_Files/xTestBinary.bin";
const std::string IN_FBX_FILEPATH = "FBX_Files/Level1[Culled].fbx";

const std::string ASCII2_FILE = "Exported_Files/Level1ASCII.txt";
const std::string BINARY2_FILE = "Exported_Files/Level1[Culled].meh";
//const std::string IN_FBX_FILEPATH = "FBX_Files/LevelTest.fbx";

// MM: Standard names, in case you mess them up too much
//const std::string ASCII_FILE = "Exported_Files/xTestAscii.txt";
//const std::string BINARY_FILE = "Exported_Files/xTestBinary.bin";
//const std::string IN_FBX_FILEPATH = "FBX_Files/cube.fbx";

struct PhongMaterial 
{
	float ambient[3];
	float diffuse[3];
	float specular[3];
	float emissive[3];
	float opacity;
	float shininess;
	float reflectivity;
	int nrOfTextures;
};

struct PhongMaterial2
{
	char name[256];
	float ambient[3];
	float diffuse[3];
	float specular[3];
	float emissive[3];
	float opacity;

	char albedo[256];
	char normal[256];
};

struct GeoTransformations
{
	float translation[3];
	float rotation[3];
	float scale[3];
};

struct Vertex
{
	float position[3];
	float uv[2];
	float normal[3];
	float tangent[3];
	float bitangent[3];
};

struct Mesh
{
	char name[256];
	char materialName[256];
	//char childName;

	unsigned int vertexCount;
};

struct MeshHolder
{
	char name[256];
	char materialName[256];
	vector<MeshHolder> children;

	// Might make this into a vector
	//vector<Vertex> vertices;
	unsigned int vertexCount;
	Vertex* vertices;

	// Constructor that may not be needed
	MeshHolder()
	{
		vertices = nullptr;
	}
};
