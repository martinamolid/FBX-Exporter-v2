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

const std::string IN_FBX_FILEPATH = "FBX_Files/BedRoomTest.fbx";
const std::string ASCII_FILE = "Exported_Files/BedRoomTestAscii.txt";
const std::string BINARY_FILE = "Exported_Files/BedRoomTest.meh";
//const std::string IN_FBX_FILEPATH = "FBX_Files/BedRoomTest.fbx";
//const std::string ASCII_FILE = "Exported_Files/TestAscii.txt";
//const std::string BINARY_FILE = "Exported_Files/TestFile.meh";

// File header
struct MehHeader
{
	int meshCount;
	int groupCount;
	int materialCount;
	int pointLightCount;
	int dirLightCount;
};

// Grouping
struct Group	// Type 0;
{
	char name[256];

	float translation[3];
	float rotation[3];
	float scale[3];

	bool isChild;
	char parentName[256];
	int parentType;
};

// Mesh data
struct Mesh		// Type 1;
{
	char name[256];
	unsigned int materialID;

	float translation[3];
	float rotation[3];
	float scale[3];

	bool isChild;
	char parentName[256];
	int parentType;

	Skeleton skeleton;

	int type;
	int link;

	unsigned int vertexCount;
};

// Vertex data
struct Vertex
{
	float position[3];
	float uv[2];
	float normal[3];
	float tangent[3];
	float bitangent[3];

	float bone[4];
	float weight[4];
};

struct PhongMaterial
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

// Skeleton data
struct Skeleton
{
	char name[256];
	int jointCount;
	int aniCount;
};

// Joint data
struct Joint
{
	char name[256];
	int parentIndex;
	float invBindPose[16];
};

// Animation data
struct Animation
{
	char name[256];
	int keyframeFirst;
	int keyframeLast;
	float duration;
	float rate;
	int keyframeCount;
};

// Keyframe data
struct KeyFrame
{
	int id = 0;

	// local transform per joint
	float transform[16];
	float rotate[16];
	float scale[16];
};

// Light data (directional)
struct DirLight 
{
	float position[3];
	float rotation[3];
	float color[3];
	float intensity;
};

// Light data (point)
struct PointLight 
{
	float position[3];
	float color[3];
	float intensity;
};

//
//
// =============== Temporary fbx data ===============
struct MeshHolder
{
	char name[256];
	char materialName[256];
	float translation[3];
	float rotation[3];
	float scale[3];

	bool isChild;
	char parentName[256];
	int parentType;

	// Might make this into a vector
	//vector<Vertex> vertices;
	int vertexCount;
	Vertex* vertices;

	int type;
	int link;

	// Constructor that may not be needed
	MeshHolder()
	{
		vertices = nullptr;
	}

	unsigned int materialID;
};

struct Skeleton
{
	struct Joint
	{
		string name;
		int parentIndex = -1;
		FbxAMatrix invBindPose;
	};

	struct Animation
	{
		struct KeyFrame
		{
			int id = 0;
			// global transform per joint (could be used if no interpolation is needed!
			vector<FbxVector4>		global_joints_T;
			vector<FbxQuaternion>	global_joints_R;
			vector<FbxVector4>		global_joints_S;

			// local transform, good for interpolation and then making a final global.
			vector<FbxVector4>		local_joints_T;
			vector<FbxQuaternion>	local_joints_R;
			vector<FbxVector4>		local_joints_S;
		};

		string name;
		int first_kf;
		int last_kf;
		float duration;
		float rate;
		vector<KeyFrame> keyframes;
	};

	string name;
	vector<Joint> joints;
	vector<Animation> animations;
};
// =============== Temporary fbx data ===============