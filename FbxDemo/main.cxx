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
	// Martina Molid
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
int PrintContent(FbxNode* pNode, MeshHolder* mesh, vector<PhongMaterial>& mats);
void DisplayTransformPropagation(FbxNode* pNode);
void PrintGeometricTransform(FbxNode* pNode);
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
    // Load the scene.
	
	// MM: This opens the .fbx file to be read from
	FbxString lFilePath("");

	if( lFilePath.IsEmpty() )
	{
		lFilePath = IN_FBX_FILEPATH.c_str();
		lResult = LoadScene(lSdkManager, fileScene, lFilePath.Buffer());
        //lResult = false;
	}

	/*========================================================================================================================	
		This is where main calls all major printing functions.
		All void Display-- functions are part of the original FBX SDK, while the string Print-- functions are re-worked versions of the Display-- functions,
			which returns a string with the information into the ASCII file and prints binary into the binary file.
		The out-commented things below are leftovers from the FBX SDK, but is a good guide as to where you might want to implement upcoming things.
		// Martina Molid
	========================================================================================================================*/


	// The root scene node that contains all the elements
	FbxNode* sceneRootNode = fileScene->GetRootNode();

	// Elements in scene (including childs of childs when true)
	unsigned int elementCount = sceneRootNode->GetChildCount(true);

	// Vector of all the meshes in the scene
	MehHeader fileHeader;
	vector<MeshHolder> meshData;
	vector<Mesh> meshes;
	vector<PhongMaterial> materials;

	if (sceneRootNode)
	{
		for (unsigned int i = 0; i < elementCount; i++)
		{
			MeshHolder fillMesh;
			int type = PrintContent(sceneRootNode->GetChild(i), &fillMesh, materials);

			switch (type)
			{
			case 1:
				meshData.push_back(fillMesh);
				break;
			case 2:
				// Light									// TODO **********************
				// *Add light filling
				break;
			case 3:
				// Light
				break;
			case 4:
				// Something
				break;
			case 5:
				// Something
				break;

			default:
				break;
			}
				
		}
	}

	// Parse writing data
	fileHeader.meshCount = (int)meshData.size();
	fileHeader.meshGroupCount = 0;
	fileHeader.materialCount = (int)materials.size();
	fileHeader.pointLightCount = 0;
	fileHeader.dirLightCount = 0;

	for (int i = 0; i < meshData.size(); i++)
	{
		Mesh fillMesh;

		// Parse name
		for (int j = 0; j < NAME_SIZE; j++)
			fillMesh.name[j] = meshData[i].name[j];
		for (int j = 0; j < NAME_SIZE; j++)
			fillMesh.materialName[j] = meshData[i].materialName[j];

		fillMesh.transformation[0] = 0.0f;
		fillMesh.transformation[1] = 0.0f;
		fillMesh.transformation[2] = 0.0f;

		fillMesh.type = meshData[i].type;
		fillMesh.link = meshData[i].link;
		fillMesh.vertexCount = meshData[i].vertexCount;

		meshes.push_back(fillMesh);
	}


	// ===== Ascii debug file =====
	// This file is only for debugging purposes and is used to read and compare the data to the binary data.
	// Note that the binary could in some cases be correct but the data here could be wrong or not updated and vice verse.
	// This should be in 100% sync with what is printed to the binary file at all times for debugging to be accurate.
	// Everything noted as *Binary data is what is going to be written to the binary file later on. Everything else are comments or debug information.
	ofstream asciiFile2;
	asciiFile2.open(ASCII_FILE);	// MM: Opens the ASCII file to write the ASCII strings to
	asciiFile2 << fixed << setprecision(10) ;

	// - 1 File header
	asciiFile2 << "  //v File Header --------------------" << endl;
	asciiFile2 << "  # Mesh count [(int)]" << endl;
	asciiFile2 << meshes.size() << endl;				//* Binary data

	asciiFile2 << "  # Material count [(int)]" << endl;
	asciiFile2 << materials.size() << endl;				//* Binary data
	asciiFile2 << "  //^ File Header --------------------" << endl << endl;
	// - 2 Meshes
	for (int i = 0; i < meshes.size(); i++)
	{
		asciiFile2 << "    //v Mesh " << i << " Header " << " --------------------" << endl << endl;

		// 1 Mesh name
		asciiFile2 << "  # Mesh name [(char) * 256]: " << endl;
		asciiFile2 << meshes[i].name << endl;			//* Binary data

		// 2  Material name
		asciiFile2 << "  # Material name [(char) * 256]: " << endl;		
		asciiFile2 << meshes[i].materialName << endl;	//* Binary data

		// 3  Entity name
		asciiFile2 << "  # Attribute type [(int)]: " << endl;
		asciiFile2 << meshes[i].type << endl;	//* Binary data

		// 4  Entity link
		asciiFile2 << "  # Attribute link [(int)]: " << endl;
		asciiFile2 << meshes[i].link << endl;	//* Binary data

		// 5 Vertex count
		asciiFile2 << "  # Vertex count [(int)]: " << endl;
		asciiFile2 << meshes[i].vertexCount << endl;	//* Binary data

		asciiFile2 << "    //^ Mesh " << i << " Header " <<  " --------------------" << endl << endl;
		
		// 4  Vertex data
		for (int j = 0; j < meshData[i].vertexCount; j++)
		{
			asciiFile2 << "  * " << j << " Vertex position / " << "uv / " << "normal / " << "tangent / " << "binormal " << "[(float) * 14]" << endl;
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

	// - 3 Materials
	for (int i = 0; i < materials.size(); i++)
	{
		asciiFile2 << "    // Material " << i << " --------------------" << endl;

		// 1 Material name
		asciiFile2 << "  # Material name [(char) * 256]: " << endl;
		asciiFile2 << materials[i].name << endl;	//* Binary data

		// 2 Material data
		asciiFile2 << "  # Ambient, diffuse, specular, emissive, opacity" << "[(float) * 13]" << endl;
		//*v Binary data
		asciiFile2 << (float)materials[i].ambient[0] << ", " << (float)materials[i].ambient[1] << ", " << (float)materials[i].ambient[2] << endl;
		asciiFile2 << (float)materials[i].diffuse[0] << ", " << (float)materials[i].diffuse[1] << ", " << (float)materials[i].diffuse[2] << endl;
		asciiFile2 << (float)materials[i].specular[0] << ", " << (float)materials[i].specular[1] << ", " << (float)materials[i].specular[2] << endl;
		asciiFile2 << (float)materials[i].emissive[0] << ", " << (float)materials[i].emissive[1] << ", " << (float)materials[i].emissive[2] << endl;
		asciiFile2 << (float)materials[i].opacity << endl;
		//*^ Binary data

		// 3 Albedo filename
		asciiFile2 << "  # Albedo name [(char) * 256]: " << endl;
		asciiFile2 << materials[i].albedo << endl;	//* Binary data

		// 4 Normal filename
		asciiFile2 << "  # Normal name [(char) * 256]: " << endl;
		asciiFile2 << materials[i].normal << endl;	//* Binary data

	}

	// - 4 Lights
	// *Add light ascii writing
	// Swap meshes size for light vector size or kaputt										// TODO **********************
	for (int i = 0; i < meshes.size(); i++)
	{
		asciiFile2 << "    // Light " << i << " --------------------" << endl;

		// 1 Light name
		asciiFile2 << "  # Light name [(char) * 256]: " << endl;
		//asciiFile2 << *name* << endl;	//* Binary data

		// 2 Light data
		//asciiFile2 << "  # Position, rotation, strength etc***************** << "[(float) * -number-]" << endl;
		//*v Binary data (visual)
		//asciiFile2 << (float)light.something[0] << ", " << (float)light.something[0] << ", " << (float)light.something[0] << endl;

	}

	asciiFile2.close();

	// ===== Binary file file =====
	// This is used to directly write binary data to the file
	// Binary data is as expected hard to read and isn't formated in a readableway even when turned into
	// it's relevant type when read (ex int, float, etc). It's up to the reader to know how it was formated
	// and format it in the same way upon reading.
	ofstream binFile2(BINARY_FILE, ofstream::binary);

	// - 1 File Header
	unsigned int meshAmount = (unsigned int)meshes.size();
	binFile2.write((char*)&meshAmount, sizeof(unsigned int));
	unsigned int materialAmount = (unsigned int)materials.size();
	binFile2.write((char*)&materialAmount, sizeof(unsigned int));

	// - 2 Meshes
	for (unsigned int i = 0; i < meshAmount; i++)
	{
		// 1 Mesh header
		binFile2.write((char*)&meshes[i], sizeof(Mesh));

		// 2 Vertex data (pos, uv, norm, tangent, bitangent)
		binFile2.write((char*)meshData[i].vertices, sizeof(Vertex) * meshes[i].vertexCount);
	}

	// - 3 Materials
	for (int i = 0; i < materials.size(); i++)
	{
		binFile2.write((char*)&materials[i], sizeof(PhongMaterial));
	}

	// - 4 Light
	// *Add light binary writing																	// TODO **********************
	for (int i = 0; i < meshes.size(); i++)
	{
		//binFile2.write((char*)&*--LightElement--, sizeof(--size--));
	}
	binFile2.close();





    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);

	system("pause");
    return 0;
}

/* 
========================================================================================================================

	PrintContent recursively prints all information in a node (and its children), determined by the type of the node.
	For now, only meshes are printed out, but all original code from the FBX SDK is left in here as a reference
		to how and where they print skeletons and lights.

	// Martina Molid

========================================================================================================================
*/

int PrintContent(FbxNode* pNode, MeshHolder* mesh, vector<PhongMaterial>& mats)
{
	// This will check what type this node is
	// All the cases represent the different types
	FbxNodeAttribute::EType lAttributeType;
	int type = 0;
	// ================

	FbxVector4 translation = pNode->EvaluateLocalTranslation();
	FbxVector4 rotation = pNode->EvaluateLocalRotation();
	FbxVector4 scale = pNode->EvaluateLocalScaling();

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

		case FbxNodeAttribute::eSkeleton:
			//DisplaySkeleton(pNode);
			break;

		case FbxNodeAttribute::eMesh:
			// This applies the relevant type (1 = mesh) and adds
			// The relevant transformation data
			type = 1;
			GetMesh(pNode, mesh, mats);
			mesh->translation[0] = (float)translation[0];
			mesh->translation[1] = (float)translation[1];
			mesh->translation[2] = (float)translation[2];
			mesh->rotation[2] = (float)rotation[2];
			mesh->rotation[2] = (float)rotation[2];
			mesh->rotation[2] = (float)rotation[2];
			mesh->scale[2] = (float)scale[2];
			mesh->scale[2] = (float)scale[2];
			mesh->scale[2] = (float)scale[2];
			break;

		case FbxNodeAttribute::eCamera:
			//DisplayCamera(pNode);
			break;

		case FbxNodeAttribute::eLight:
			type = 2;
			// *Add light functions
			// *Add light position and rotation									// TODO **********************
			break;

		}
	}

	// Loops through all the children of this node
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		MeshHolder fillMesh;
		
		int ctype = PrintContent(pNode->GetChild(i), &fillMesh, mats);

		switch (ctype)
		{
		case 1:
			mesh->children.push_back(fillMesh);
			break;
		case 2:
			// Light
			// * Add light push_back											// TODO **********************
			break;
		case 3:
			// Light
			break;
		case 4:
			// Something
			break;
		case 5:
			// Something
			break;

		default:
			break;

		}
	}



	return type;
}


void DisplayTransformPropagation(FbxNode* pNode)
{
    FBXSDK_printf("    Transformation Propagation\n");

    // 
    // Rotation Space
    //
    EFbxRotationOrder lRotationOrder;
    pNode->GetRotationOrder(FbxNode::eSourcePivot, lRotationOrder);

    FBXSDK_printf("        Rotation Space: ");

    switch (lRotationOrder)
    {
    case eEulerXYZ: 
        FBXSDK_printf("Euler XYZ\n");
        break;
    case eEulerXZY:
        FBXSDK_printf("Euler XZY\n");
        break;
    case eEulerYZX:
        FBXSDK_printf("Euler YZX\n");
        break;
    case eEulerYXZ:
        FBXSDK_printf("Euler YXZ\n");
        break;
    case eEulerZXY:
        FBXSDK_printf("Euler ZXY\n");
        break;
    case eEulerZYX:
        FBXSDK_printf("Euler ZYX\n");
        break;
    case eSphericXYZ:
        FBXSDK_printf("Spheric XYZ\n");
        break;
    }

    //
    // Use the Rotation space only for the limits
    // (keep using eEulerXYZ for the rest)
    //
    FBXSDK_printf("        Use the Rotation Space for Limit specification only: %s\n",
        pNode->GetUseRotationSpaceForLimitOnly(FbxNode::eSourcePivot) ? "Yes" : "No");


    //
    // Inherit Type
    //
    FbxTransform::EInheritType lInheritType;
    pNode->GetTransformationInheritType(lInheritType);

    FBXSDK_printf("        Transformation Inheritance: ");

    switch (lInheritType)
    {
    case FbxTransform::eInheritRrSs:
        FBXSDK_printf("RrSs\n");
        break;
    case FbxTransform::eInheritRSrs:
        FBXSDK_printf("RSrs\n");
        break;
    case FbxTransform::eInheritRrs:
        FBXSDK_printf("Rrs\n");
        break;
    }
}


/*
========================================================================================================================

	PrintGeometricTransform gets the object's geometric transformations and prints it to the ASCII and binary file.
		The problem right now is that it has to be called before or after the node printing.

	// Martina Molid

========================================================================================================================
*/
void PrintGeometricTransform(FbxNode* pNode)
{
    FbxVector4 lTmpVector;
	Transformation transformation;

    // Translation
    lTmpVector = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	transformation.translation[0] = (float)lTmpVector[0];
	transformation.translation[1] = (float)lTmpVector[1];
	transformation.translation[2] = (float)lTmpVector[2];

    // Rotation
    lTmpVector = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	transformation.rotation[0] = (float)lTmpVector[0];
	transformation.rotation[1] = (float)lTmpVector[1];
	transformation.rotation[2] = (float)lTmpVector[2];

    // Scaling
    lTmpVector = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
	transformation.scale[0] = (float)lTmpVector[0];
	transformation.scale[1] = (float)lTmpVector[1];
	transformation.scale[2] = (float)lTmpVector[2];
}