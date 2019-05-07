/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/


/*========================================================================================================================
	This is the main file of the FBX to Custom Binary Converter for Group 3 in UD1446: Small Game Project
	To decide the filenames for output, see Filenames.h
========================================================================================================================*/

// MM: The Common.h include has a lot of the FBX SDK defined functions and types like FBXSDK_printf
#include "../Common/Common.h"

#include "Filenames.h"
#include "PrintMesh.h"
#include "PrintNrOfMeshes.h"
#include "PrintLight.h"

#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;

#pragma comment(lib,"libfbxsdk-mt.lib")
#pragma comment(lib,"libxml2-mt.lib")
#pragma comment(lib,"zlib-mt.lib")

// Local function prototypes.
void PrintContent(FbxNode* pNode, vector<Group>& fillGroup, vector<MeshHolder>& mesh, vector<PhongMaterial>& mats, bool isChild, int parentType);
void DisplayPivotsAndLimits(FbxNode* pNode);


int main(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* fileScene = NULL;
    bool lResult;

	// In commandline parameters
	vector<string> inParameters;
	for (int i = 0; i < argc; i++)
	{
		inParameters.push_back(argv[i]);
	}

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, fileScene);

	// This is wherethe in fbx filepath should be added comming in from the CMD
	FbxString lFilePath("");
	lFilePath = IN_FBX_FILEPATH.c_str();
	// Load the scene.
	lResult = LoadScene(lSdkManager, fileScene, lFilePath.Buffer());


	//	===== Data collection ==================================================
	//	This is where all the data from the FBX scene is collected.
	//	The data is then stored in the loader while it's parsed and written
	//	to the custom format
	//	========================================================================

	// The root scene node that contains all the elements
	FbxNode* sceneRootNode = fileScene->GetRootNode();

	// Elements in scene (including childs of childs when true)
	int elementCount = sceneRootNode->GetChildCount();

	// Vectors of elements in the scene
	vector<MeshHolder> meshData;

	MehHeader* fileHeader = new MehHeader;
	vector<Group> groups;
	vector<Mesh> meshes;
	vector<PhongMaterial> materials;

	if (sceneRootNode)
	{
		for (int i = 0; i < elementCount; i++)
		{
			PrintContent(sceneRootNode->GetChild(i), groups, meshData, materials, false, -1);		
		}
	}

	//	===== Parse data ==================================================
	//	This section will parse and data that isn't yet fully loaded in for 
	//	the writing of the custom file
	//	===================================================================

	// ==== Header ====
	fileHeader->meshCount = (int)meshData.size();
	fileHeader->groupCount = (int)groups.size();;
	fileHeader->materialCount = (int)materials.size();
	fileHeader->pointLightCount = 0;											// TODO **********************
	fileHeader->dirLightCount = 0;											// TODO **********************

	// ==== Meshes ====
	for (int i = 0; i < meshData.size(); i++)
	{
		Mesh fillMesh;

		// Name
		for (int j = 0; j < NAME_SIZE; j++)
			fillMesh.name[j] = meshData[i].name[j];
		// Material
		for (int j = 0; j < NAME_SIZE; j++)
			fillMesh.materialName[j] = meshData[i].materialName[j];
		// Transformation
		fillMesh.translation[0] = meshData[i].translation[0];
		fillMesh.translation[1] = meshData[i].translation[1];
		fillMesh.translation[2] = meshData[i].translation[2];
		fillMesh.rotation[0] = meshData[i].rotation[0];
		fillMesh.rotation[1] = meshData[i].rotation[1];
		fillMesh.rotation[2] = meshData[i].rotation[2];
		fillMesh.scale[0] = meshData[i].scale[0];
		fillMesh.scale[1] = meshData[i].scale[1];
		fillMesh.scale[2] = meshData[i].scale[2];
		// Hierarchy
		fillMesh.isChild = meshData[i].isChild;

		int nameLength = (int)strlen(meshData[i].parentName);
		for (int j = 0; j < nameLength + 1; j++)
			fillMesh.parentName[j] = meshData[i].parentName[j];
		// Puts a \0 at the end of the mesh name, still printing out whitespace into the binary file

		fillMesh.parentType = meshData[i].parentType;
		// Custom attribute
		fillMesh.type = meshData[i].type;
		fillMesh.link = meshData[i].link;
		// Vertex count
		fillMesh.vertexCount = meshData[i].vertexCount;
		meshes.push_back(fillMesh);
	}


	// ===== Ascii debug file ==================================================
	// This file is only for debugging purposes and is used to read and compare the data to the binary data.
	// Note that the binary could in some cases be correct but the data here could be wrong or not updated and vice verse.
	// This should be in 100% sync with what is printed to the binary file at all times for debugging to be accurate.
	// Everything noted as *Binary data is what is going to be written to the binary file later on. Everything else are comments or debug information.
	//	========================================================================
	ofstream asciiFile2;
	asciiFile2.open(ASCII_FILE);	// This is where out the filepath should be added comming in from the CMD
	asciiFile2 << fixed << setprecision(10) ;

	// - 1 File header
	asciiFile2 << "  //v File Header --------------------" << endl;
	asciiFile2 << "  # Mesh count" << endl;
	asciiFile2 << fileHeader->meshCount << endl;					//* Binary data
	asciiFile2 << "  # Mesh Group count" << endl;
	asciiFile2 << fileHeader->groupCount << endl;				//* Binary data
	asciiFile2 << "  # Material count" << endl;
	asciiFile2 << fileHeader->materialCount << endl;				//* Binary data
	asciiFile2 << "  # Point Light count" << endl;
	asciiFile2 << fileHeader->pointLightCount << endl;			//* Binary data
	asciiFile2 << "  # Directional count" << endl;
	asciiFile2 << fileHeader->dirLightCount << endl;				//* Binary data
	asciiFile2 << "  //^ File Header --------------------" << endl << endl;

	// - 2 Groups
	for (int i = 0; i < fileHeader->groupCount; i++)
	{
		asciiFile2 << "    //v Group " << i << " --------------------" << endl << endl;

		// 2.1 Group name
		asciiFile2 << "  # Group name: " << endl;
		asciiFile2 << groups[i].name << endl;					//* Binary data
		// 2.2 Transformation

		asciiFile2 << "  * "  << "Translation vector: " << endl;
		//v Binary data
		asciiFile2 << (float)groups[i].translation[0] << ", " << (float)groups[i].translation[1] << ", " << (float)groups[i].translation[2] << endl;

		asciiFile2 << "  * "  << "Rotation vector: " << endl;
		//v Binary data
		asciiFile2 << (float)groups[i].rotation[0] << ", " << (float)groups[i].rotation[1] << ", " << (float)groups[i].rotation[2] << endl;

		asciiFile2 << "  * " << "Scale vector: " << endl;
		//v Binary data
		asciiFile2 << (float)groups[i].scale[0] << ", " << (float)groups[i].scale[1] << ", " << (float)groups[i].scale[2] << endl;

		// 2.3 Hierarchy
		asciiFile2 << "  # Is child: " << endl;
		asciiFile2 << groups[i].isChild << endl;				//* Binary data
		asciiFile2 << "  # Parent Name: " << endl;
		asciiFile2 << groups[i].parentName << endl;				//* Binary data
		asciiFile2 << "  # Parent type: " << endl;
		asciiFile2 << groups[i].parentType << endl;				//* Binary data

		asciiFile2 << "    //^ Group " << i << " --------------------" << endl << endl;
		asciiFile2 << endl;
	}

	// - 3 Meshes
	for (int i = 0; i < fileHeader->meshCount; i++)
	{
		asciiFile2 << "    //v Mesh " << i << " Header " << " --------------------" << endl << endl;

		// 3.1 Mesh name
		asciiFile2 << "  # Mesh name: " << endl;
		asciiFile2 << meshes[i].name << endl;					//* Binary data
		// 3.2  Material name
		asciiFile2 << "  # Material name : " << endl;		
		asciiFile2 << meshes[i].materialName << endl;			//* Binary data
		// 3.3 Transformation
		asciiFile2 << "  * " << "Translation vector: " << endl;
		asciiFile2 << (float)meshData[i].translation[0] << ", " << (float)meshData[i].translation[1] << ", " << (float)meshData[i].translation[2] << endl; 	//* Binary data
		asciiFile2 << "  * " << "Rotation vector: " << endl;
		asciiFile2 << (float)meshData[i].rotation[0] << ", " << (float)meshData[i].rotation[1] << ", " << (float)meshData[i].rotation[2] << endl; 			//* Binary data
		asciiFile2 << "  * " << "Scale vector: " << endl;
		asciiFile2 << (float)meshData[i].scale[0] << ", " << (float)meshData[i].scale[1] << ", " << (float)meshData[i].scale[2] << endl; 					//* Binary data
		// 3.4 hierarchy
		asciiFile2 << "  # Is child: " << endl;
		asciiFile2 << meshes[i].isChild << endl;				//* Binary data
		asciiFile2 << "  # Parent Name: " << endl;
		asciiFile2 << meshes[i].parentName << endl;				//* Binary data
		asciiFile2 << "  # Parent type: " << endl;
		asciiFile2 << meshes[i].parentType << endl;				//* Binary data
		// 3.5 Entity attributes
		asciiFile2 << "  # Attribute type: " << endl;
		asciiFile2 << meshes[i].type << endl;					//* Binary data
		asciiFile2 << "  # Attribute link: " << endl;
		asciiFile2 << meshes[i].link << endl;					//* Binary data
		// 3.6 Vertex count
		asciiFile2 << "  # Vertex count: " << endl;
		asciiFile2 << meshes[i].vertexCount << endl;			//* Binary data

		asciiFile2 << "    //^ Mesh " << i << " Header " <<  " --------------------" << endl << endl;
		
		// 3.* Vertex data
		for (int j = 0; j < meshData[i].vertexCount; j++)
		{
			asciiFile2 << "  * " << j << " Vertex position / " << "uv / " << "normal / " << "tangent / " << "binormal " << endl;
			//v Binary data
			asciiFile2 << (float)meshData[i].vertices[j].position[0] << ", "	<< (float)meshData[i].vertices[j].position[1] << ", "		<< (float)meshData[i].vertices[j].position[2] << endl; 
			asciiFile2 << (float)meshData[i].vertices[j].uv[0] << ", "		<< (float)meshData[i].vertices[j].uv[1] << ", "			<< endl;
			asciiFile2 << (float)meshData[i].vertices[j].normal[0] << ", "	<< (float)meshData[i].vertices[j].normal[1] << ", "		<< (float)meshData[i].vertices[j].normal[2] << endl;
			asciiFile2 << (float)meshData[i].vertices[j].tangent[0] << ", "	<< (float)meshData[i].vertices[j].tangent[1] << ", "		<< (float)meshData[i].vertices[j].tangent[2] << endl;
			asciiFile2 << (float)meshData[i].vertices[j].bitangent[0] << ", "	<< (float)meshData[i].vertices[j].bitangent[1] << ", "	<< (float)meshData[i].vertices[j].bitangent[2] << endl << endl;
			//^ Binary data
		}
		asciiFile2 << endl;
	}
	// - 5 Materials
	for (int i = 0; i < fileHeader->materialCount; i++)
	{
		asciiFile2 << "    // Material " << i << " --------------------" << endl;

		// 5.1 Material name
		asciiFile2 << "  # Material name: " << endl;
		asciiFile2 << materials[i].name << endl;				//* Binary data

		// 5.2 Material data
		asciiFile2 << "  # Ambient, diffuse, specular, emissive, opacity" << endl;
		//*v Binary data
		asciiFile2 << (float)materials[i].ambient[0] << ", " << (float)materials[i].ambient[1] << ", " << (float)materials[i].ambient[2] << endl;
		asciiFile2 << (float)materials[i].diffuse[0] << ", " << (float)materials[i].diffuse[1] << ", " << (float)materials[i].diffuse[2] << endl;
		asciiFile2 << (float)materials[i].specular[0] << ", " << (float)materials[i].specular[1] << ", " << (float)materials[i].specular[2] << endl;
		asciiFile2 << (float)materials[i].emissive[0] << ", " << (float)materials[i].emissive[1] << ", " << (float)materials[i].emissive[2] << endl;
		asciiFile2 << (float)materials[i].opacity << endl;
		//*^ Binary data

		// 5.3 Albedo filename
		asciiFile2 << "  # Albedo name: " << endl;
		asciiFile2 << materials[i].albedo << endl;				//* Binary data

		// 5.4 Normal filename
		asciiFile2 << "  # Normal name: " << endl;
		asciiFile2 << materials[i].normal << endl;				//* Binary data
	}
	// - 6 Lights
	// *Add light ascii writing (1 forloop for each type, copy this one for more light types)
	// Swap meshes size for light vector size or kaputt														// TODO **********************
	for (int i = 0; i < fileHeader->meshCount; i++)
	{
		asciiFile2 << "    // Light " << i << " --------------------" << endl;

		// 1 Light name
		asciiFile2 << "  # Light name: " << endl;
		//asciiFile2 << *name* << endl;	//* Binary data

		// 2 Light data
		//asciiFile2 << "  # Position, rotation, strength etc***************** << "[(float) * -number-]" << endl;
		//*v Binary data (visual)
		//asciiFile2 << (float)light.something[0] << ", " << (float)light.something[0] << ", " << (float)light.something[0] << endl;

	}
	asciiFile2.close();

	
	// ===== Binary file file ==================================================
	// This is used to directly write binary data to the file
	// Binary data is as expected hard to read and isn't formated in a readableway even when turned into
	// it's relevant type when read (ex int, float, etc). It's up to the reader to know how it was formated
	// and format it in the same way upon reading.
	//	========================================================================
	ofstream binaryFile(BINARY_FILE, ofstream::binary);	// This is where out the filepath should be added comming in from the CMD
	// - 1 File Header
	binaryFile.write((char*)&fileHeader, sizeof(MehHeader));
	// - 2 Groups
	for (int i = 0; i < fileHeader->groupCount; i++)
	{
		// 1 Mesh header
		binaryFile.write((char*)&groups[i], sizeof(Group));
	}

	// - 3 Meshes
	for (int i = 0; i < fileHeader->meshCount; i++)
	{
		// 3.1 Mesh header
		binaryFile.write((char*)&meshes[i], sizeof(Mesh));

		// 3.2 Vertex data (pos, uv, norm, tangent, bitangent)
		binaryFile.write((char*)meshData[i].vertices, sizeof(Vertex) * meshes[i].vertexCount);
	}

	// - 4 Materials
	for (int i = 0; i < fileHeader->materialCount; i++)
	{
		binaryFile.write((char*)&materials[i], sizeof(PhongMaterial));
	}

	// - 4 Light
	// *Add light binary writing (1 forloop for each type, copy this one for more light types)				// TODO **********************
	// Change meshCount to lights or kaputt
	for (int i = 0; i < fileHeader->meshCount; i++)
	{
		//binFile2.write((char*)&*--LightElement--, sizeof(--size--));
	}
	binaryFile.close();



    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);

	system("pause");
    return 0;
}

/*========================================================================================================================
	PrintContent recursively prints all information in a node (and its children), determined by the type of the node.
========================================================================================================================*/
void PrintContent(FbxNode* pNode, vector<Group>& groups, vector<MeshHolder>& meshes, vector<PhongMaterial>& mats, bool isChild, int parentType)
{
	// This will check what type this node is
	// All the cases represent the different types
	FbxNodeAttribute::EType lAttributeType;

	FbxVector4 translation = pNode->EvaluateLocalTranslation();
	FbxVector4 rotation = pNode->EvaluateLocalRotation();
	FbxVector4 scale = pNode->EvaluateLocalScaling();

	int nameLength = (int)strlen(pNode->GetName());
	string nameBuffer = pNode->GetName();
	int pNameLength = (int)strlen(pNode->GetParent()->GetName());
	string pNameBuffer = pNode->GetParent()->GetName();
	
	MeshHolder fillMesh;
	if (pNode->GetNodeAttribute() == NULL)
	{
		FBXSDK_printf("NULL Node Attribute\n\n");
	}
	else
	{
		lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

		switch (lAttributeType)
		{
		default:
			break;

		case FbxNodeAttribute::eNull:
			// This is probably a group
			Group fillGroup;
			// Applies the mesh name
			for (int j = 0; j < nameLength; j++)
				fillGroup.name[j] = nameBuffer[j];
			fillGroup.name[nameLength] = '\0';

			fillGroup.translation[0] = (float)translation[0];
			fillGroup.translation[1] = (float)translation[1];
			fillGroup.translation[2] = (float)translation[2];
			fillGroup.rotation[0] = (float)rotation[0];
			fillGroup.rotation[1] = (float)rotation[1];
			fillGroup.rotation[2] = (float)rotation[2];
			fillGroup.scale[0] = (float)scale[0];
			fillGroup.scale[1] = (float)scale[1];
			fillGroup.scale[2] = (float)scale[2];
			fillGroup.isChild = isChild;
			for (int j = 0; j < pNameLength; j++)
				fillGroup.parentName[j] = pNameBuffer[j];
			fillGroup.parentName[pNameLength] = '\0';
				// Puts a \0 at the end of the mesh name, still printing out whitespace into the binary file
			fillGroup.parentType = parentType;
			
			parentType = 0;
			groups.push_back(fillGroup);
			break;

		case FbxNodeAttribute::eSkeleton:
			//DisplaySkeleton(pNode);
			break;

		case FbxNodeAttribute::eMesh:
			// This applies the relevant type (1 = mesh) and adds
			// The relevant transformation data
			fillMesh.translation[0] = (float)translation[0];
			fillMesh.translation[1] = (float)translation[1];
			fillMesh.translation[2] = (float)translation[2];
			fillMesh.rotation[0] = (float)rotation[0];
			fillMesh.rotation[1] = (float)rotation[1];
			fillMesh.rotation[2] = (float)rotation[2];
			fillMesh.scale[0] = (float)scale[0];
			fillMesh.scale[1] = (float)scale[1];
			fillMesh.scale[2] = (float)scale[2];
			GetMesh(pNode, &fillMesh, mats);
			fillMesh.isChild = isChild;
			for (int j = 0; j < pNameLength; j++)
				fillMesh.parentName[j] = pNameBuffer[j];
			fillMesh.parentName[pNameLength] = '\0';
			// Puts a \0 at the end of the mesh name, still printing out whitespace into the binary file

			fillMesh.parentType = parentType;

			parentType = 1;
			meshes.push_back(fillMesh);
			break;

		case FbxNodeAttribute::eCamera:
			//DisplayCamera(pNode);
			break;

		case FbxNodeAttribute::eLight:
			// *Add light functions
			// *Add light position and rotation									// TODO **********************
			break;

		}
	}

	// Loops through all the children of this node
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		PrintContent(pNode->GetChild(i), groups, meshes, mats, true, parentType);
	}

}

