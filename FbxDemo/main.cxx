/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/

/////////////////////////////////////////////////////////////////////////
//
// This example illustrates how to detect if a scene is password 
// protected, import and browse the scene to access node and animation 
// information. It displays the content of the FBX file which name is 
// passed as program argument. You can try it with the various FBX files 
// output by the export examples.
//
/////////////////////////////////////////////////////////////////////////

// =====================================================================================================================
// =====================================================================================================================
/*
========================================================================================================================

	This is the main file of the FBX to Custom Binary Converter for Group 3 in UD1446: Small Game Project

	To decide the filenames for output, see Filenames.h

	**PLEASE, DO NOT REMOVE OUT-COMMENTED CODE UNLESS YOU ARE 120% SURE IT WILL NOT USE IT IN THE FUTURE**

	// Martina Molid

========================================================================================================================
*/

// MM: The Common.h include has a lot of the FBX SDK defined functions and types like FBXSDK_printf
#include "../Common/Common.h"
//#include "DisplayCommon.h"
//#include "DisplayHierarchy.h"
//#include "DisplayAnimation.h"
//#include "DisplayMarker.h"
//#include "DisplaySkeleton.h"
//#include "DisplayMesh.h"
//#include "DisplayNurb.h"
//#include "DisplayPatch.h"
//#include "DisplayLodGroup.h"
//#include "DisplayCamera.h"
//#include "DisplayLight.h"
//#include "DisplayGlobalSettings.h"
//#include "DisplayPose.h"
//#include "DisplayPivotsAndLimits.h"
//#include "DisplayUserProperties.h"
//#include "DisplayGenericInfo.h"

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
string PrintContent(FbxScene* pScene);
string PrintContent(FbxNode* pNode);
void PrintContent(FbxNode* pNode, Mesh* mesh, vector<PhongMaterial2>& mats);
void DisplayTarget(FbxNode* pNode);
void DisplayTransformPropagation(FbxNode* pNode);
string PrintGeometricTransform(FbxNode* pNode);
void DisplayMetaData(FbxScene* pScene);



static bool gVerbose = true;

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

	// ASCII FILE OPENING _ TO BE MOVED
	ofstream asciiFile;
	asciiFile.open(ASCII_FILE);	// MM: Opens the ASCII file to write the ASCII strings to

	/*
	========================================================================================================================
		
		This is where main calls all major printing functions.

		All void Display-- functions are part of the original FBX SDK, while the string Print-- functions are re-worked versions of the Display-- functions,
			which returns a string with the information into the ASCII file and prints binary into the binary file.

		The out-commented things below are leftovers from the FBX SDK, but is a good guide as to where you might want to implement upcoming things.

		// Martina Molid
		
	========================================================================================================================
	*/


	// INFORMATION HOLDING OF THE SCENE FOR WRITING
	FbxNode* sceneRootNode = fileScene->GetRootNode();

	// Elements in scene (including childs of childs)
	int elementCount = sceneRootNode->GetChildCount(true);

	// Vector of all the meshes in the scene
	vector<Mesh> meshes;
	vector<PhongMaterial2> materials;

	// Reserves memory
	meshes.reserve(elementCount);

	// Right now will pushback one mesh for every _element_ in the scene
	// Need to look over how to distinguish mesh from other objects
	if (sceneRootNode)
	{
		for (int i = 0; i < elementCount; i++)
		{
			Mesh fillMesh;
			PrintContent(sceneRootNode->GetChild(i), &fillMesh, materials);
			meshes.push_back(fillMesh);
		}
	}


	// ===== Ascii debug file =====
	// This file is only for debugging purposes and is used to read and compare the data to the binary data.
	// Note that the binary could in some cases be correct but the data here could be wrong or not updates.
	// This should be in 100% sync with what is printed to the binary file at all times for debugging to be accurate.
	// Everything noted as Binary data is what is written to the binary file. Everything else are comments or debug information.
	ofstream asciiFile2;
	asciiFile2.open(ASCII2_FILE);	// MM: Opens the ASCII file to write the ASCII strings to
	asciiFile2 << fixed << setprecision(5) ;

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

		// --- MM: Getting, formatting and printing mesh name ---
		int nameLength = (int)strlen(meshes[i].name.c_str());
		char finalMeshName[NAME_SIZE];
		for (int j = 0; j < nameLength; j++) 
		{
			finalMeshName[j] = meshes[i].name[j];
		}
		finalMeshName[nameLength] = '\0';				// Puts a \0 at the end of the mesh name, still printing out whitespace into the binary file

		// 1 Mesh name
		asciiFile2 << "  # Mesh name [(char) * 256]: " << endl;
		asciiFile2 << finalMeshName << endl;			//* Binary data

		// 2  Material count
		asciiFile2 << "  # Material count [(int)]: " << endl;		
		asciiFile2 << (int)meshes[i].materialNames.size() << endl;	//* Binary data

		// 3 Vertex count
		asciiFile2 << "  # Vertex count [(int)]: " << endl;
		asciiFile2 << meshes[i].vertexCount << endl;	//* Binary data

		asciiFile2 << "    //^ Mesh " << i << " Header " <<  " --------------------" << endl << endl;
		
		// 4  Vertex data
		for (int j = 0; j < meshes[i].vertexCount; j++)
		{
			asciiFile2 << "  * " << j << " Vertex position / " << "uv / " << "normal / " << "tangent / " << "binormal " << "[(float) * 14]" << endl;
			//v Binary data
			asciiFile2 << (float)meshes[i].vertices[j].position[0] << ", "	<< (float)meshes[i].vertices[j].position[1] << ", "		<< (float)meshes[i].vertices[j].position[2] << endl; 
			asciiFile2 << (float)meshes[i].vertices[j].uv[0] << ", "		<< (float)meshes[i].vertices[j].uv[1] << ", "			<< endl;
			asciiFile2 << (float)meshes[i].vertices[j].normal[0] << ", "	<< (float)meshes[i].vertices[j].normal[1] << ", "		<< (float)meshes[i].vertices[j].normal[2] << endl;
			asciiFile2 << (float)meshes[i].vertices[j].tangent[0] << ", "	<< (float)meshes[i].vertices[j].tangent[1] << ", "		<< (float)meshes[i].vertices[j].tangent[2] << endl;
			asciiFile2 << (float)meshes[i].vertices[j].bitangent[0] << ", "	<< (float)meshes[i].vertices[j].bitangent[1] << ", "	<< (float)meshes[i].vertices[j].bitangent[2] << endl << endl;
			//^ Binary data
		}
		asciiFile2 << endl;
	}

	// - 3 Materials
	for (int i = 0; i < materials.size(); i++)
	{
		asciiFile2 << "    // Material " << i << " --------------------" << endl;
		// 1 Materials index
		asciiFile2 << "  # Material index [(int)]: " << endl;
		asciiFile2 << i << endl;				//* Binary data

		// Size: 256 * char
		int nameLength = (int)strlen(meshes[i].name.c_str());
		char finalMeshName[NAME_SIZE];
		for (int j = 0; j < nameLength; j++) {
			finalMeshName[j] = meshes[i].name[j];
		}
		finalMeshName[nameLength] = '\0';		// Puts a \0 at the end of the mesh name, still printing out whitespace into the binary file
		// 2 Material name
		asciiFile2 << "  # Material name [(char) * 256]: " << endl;
		asciiFile2 << finalMeshName << endl;	//* Binary data

		// 3 Material data
		asciiFile2 << "  # Ambient, diffuse, specular, emissive, opacity, shininess, reflectivity" << "[(float) * 15]" << endl;
		//*v Binary data
		asciiFile2 << (float)materials[i].ambient[0] << ", " << (float)materials[i].ambient[1] << ", " << (float)materials[i].ambient[2] << endl;
		asciiFile2 << (float)materials[i].diffuse[0] << ", " << (float)materials[i].diffuse[1] << ", " << (float)materials[i].diffuse[2] << endl;
		asciiFile2 << (float)materials[i].specular[0] << ", " << (float)materials[i].specular[1] << ", " << (float)materials[i].specular[2] << endl;
		asciiFile2 << (float)materials[i].emissive[0] << ", " << (float)materials[i].emissive[1] << ", " << (float)materials[i].emissive[2] << endl;
		asciiFile2 << (float)materials[i].opacity << endl;
		asciiFile2 << (float)materials[i].shininess << endl;
		asciiFile2 << (float)materials[i].reflectivity << endl;
		//*^ Binary data

		asciiFile2 << "  # Texture count [(int)]: " << endl;
		asciiFile2 << (int)materials[i].nrOfTextures << endl;	//* Binary data

		// 4 Albedo filename

		// 5 Normal filename

	}
	asciiFile2.close();


	ofstream binFile2(BINARY2_FILE, ofstream::binary);

	// - 1 File Header
	int meshAmount = (int)meshes.size();
	binFile2.write((char*)&meshAmount, sizeof(int));

	int materialAmount = (int)materials.size();
	binFile2.write((char*)&materialAmount, sizeof(int));

	// - 2 Meshes
	for (int i = 0; i < meshes.size(); i++)
	{
		// --- MM: Getting, formatting and printing mesh name ---
		// Size: 256 * char
		int nameLength = (int)strlen(meshes[i].name.c_str());
		char finalMeshName[NAME_SIZE];
		for (int j = 0; j < nameLength; j++) {
			finalMeshName[j] = meshes[i].name[j];
		}
		finalMeshName[nameLength] = '\0'; // Puts a \0 at the end of the mesh name, still printing out whitespace into the binary file

		// 1 Mesh name
		binFile2.write((char*)finalMeshName, sizeof(char) * NAME_SIZE);

		// 2 Material count
		int nrOfMat = (int)meshes[i].materialNames.size();
		binFile2.write((char*)&nrOfMat, sizeof(int));

		// 3 Vertex count
		int vtxCount = meshes[i].vertexCount;
		binFile2.write((char*)&vtxCount, sizeof(int));

		// 4 Vertex data (pos, uv, norm, tangent, bitangent)
		binFile2.write((char*)meshes[i].vertices, sizeof(Vertex) * vtxCount);
	}

	// - 3 Materials
	for (int i = 0; i < materials.size(); i++)
	{
		// 1 Material index
		binFile2.write((char*)&i, sizeof(int));

		// Size: 256 * char
		int nameLength = (int)strlen(meshes[i].name.c_str());
		char finalMaterialName[NAME_SIZE];
		for (int j = 0; j < nameLength; j++) {
			finalMaterialName[j] = materials[i].name[j];
		}
		finalMaterialName[nameLength] = '\0'; // Puts a \0 at the end of the mesh name, still printing out whitespace into the binary file

		// 2 Material names
		binFile2.write((char*)&finalMaterialName, sizeof(char) * NAME_SIZE);

		// 3 Material data
		binFile2.write((char*)&materials[i], sizeof(PhongMaterial2));
		// MM: File has to be closed here as it is reopened in PrintTexture
		
	}
	binFile2.close();



	// --------------------------------------
    if(lResult == false)
    {
        FBXSDK_printf("\n\nAn error occurred while loading the scene...");
    }
    else 
    {
        // Display the scene.
        //DisplayMetaData(lScene);

        //FBXSDK_printf("\n\n---------------------\nGlobal Light Settings\n---------------------\n\n");

        //if( gVerbose ) //DisplayGlobalLightSettings(&lScene->GetGlobalSettings());

        //FBXSDK_printf("\n\n----------------------\nGlobal Camera Settings\n----------------------\n\n");

        //if( gVerbose ) //DisplayGlobalCameraSettings(&lScene->GetGlobalSettings());

        //FBXSDK_printf("\n\n--------------------\nGlobal Time Settings\n--------------------\n\n");

        //if( gVerbose ) //DisplayGlobalTimeSettings(&lScene->GetGlobalSettings());

        //FBXSDK_printf("\n\n---------\nHierarchy\n---------\n\n");

			// TO BE DELETED - OLD WAY TO GET ELEMENTS
		if (gVerbose) asciiFile << PrintNrOfMeshes(fileScene);//PrintHierarchy(lScene); // MM: Prints how many meshes exists in the FBX file

        //FBXSDK_printf("\n\n------------\nNode Content\n------------\n\n");

			
        if( gVerbose ) asciiFile << PrintContent(fileScene);	// MM: Prints node content, currently only meshes ( see below )

        //FBXSDK_printf("\n\n----\nPose\n----\n\n");

        //if( gVerbose ) //DisplayPose(lScene);

        //FBXSDK_printf("\n\n---------\nAnimation\n---------\n\n");

        //if( gVerbose ) //DisplayAnimation(lScene);

        //now display generic information

        //FBXSDK_printf("\n\n---------\nGeneric Information\n---------\n\n");
		// if( gVerbose ) {//DisplayGenericInfo(lScene);
		//}

    }

	// ASCII FILE OPENING _ TO BE MOVED
	asciiFile.close();



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
string PrintContent(FbxScene* pScene)
{
    FbxNode* lNode = pScene->GetRootNode();

	string pString;

    if(lNode)
    {
        for(int i = 0; i < lNode->GetChildCount(); i++)
        {
            pString += PrintContent(lNode->GetChild(i));
        }
    }

	return pString;
}

string PrintContent(FbxNode* pNode)
{
    FbxNodeAttribute::EType lAttributeType;
    int i;

	string pString;

    if(pNode->GetNodeAttribute() == NULL)
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
        case FbxNodeAttribute::eMarker:  
            //DisplayMarker(pNode);
            break;

        case FbxNodeAttribute::eSkeleton:  
            //DisplaySkeleton(pNode);
            break;

        case FbxNodeAttribute::eMesh:      
            //DisplayMesh(pNode);
			pString += PrintMesh(pNode);
            break;

        case FbxNodeAttribute::eNurbs:      
            //DisplayNurb(pNode);
            break;

        case FbxNodeAttribute::ePatch:     
            //DisplayPatch(pNode);
            break;

        case FbxNodeAttribute::eCamera:    
            //DisplayCamera(pNode);
            break;

        case FbxNodeAttribute::eLight:     
            //DisplayLight(pNode);
			pString += PrintLight(pNode);
            break;

        case FbxNodeAttribute::eLODGroup:
            //DisplayLodGroup(pNode);
            break;
        }   
    }

    /*DisplayUserProperties(pNode);
    DisplayTarget(pNode);
    DisplayPivotsAndLimits(pNode);
    DisplayTransformPropagation(pNode);*/
    pString += PrintGeometricTransform(pNode);

    for(i = 0; i < pNode->GetChildCount(); i++)
    {
        pString += PrintContent(pNode->GetChild(i));
		
    }

	return pString;
}


void PrintContent(FbxNode* pNode, Mesh* mesh, vector<PhongMaterial2>& mats)
{
	// This will check what type this node is
	// All the cases represent the different types
	FbxNodeAttribute::EType lAttributeType;

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
		case FbxNodeAttribute::eMarker:
			//DisplayMarker(pNode);
			break;

		case FbxNodeAttribute::eSkeleton:
			//DisplaySkeleton(pNode);
			break;

		case FbxNodeAttribute::eMesh:
			//DisplayMesh(pNode);
			
			// Here we can find out that this node is a mesh
			// We could utilize this in the vector and only push back things we know are meshes once we aquire them
			// Alternatively extract this from the swtich case since this is a template for printing information to 
			// the command prompt with no regards to format or similar
			GetMesh(pNode, mesh, mats);
			break;

		case FbxNodeAttribute::eNurbs:
			//DisplayNurb(pNode);
			break;

		case FbxNodeAttribute::ePatch:
			//DisplayPatch(pNode);
			break;

		case FbxNodeAttribute::eCamera:
			//DisplayCamera(pNode);
			break;

		case FbxNodeAttribute::eLight:
			//DisplayLight(pNode);
			//PrintLight(pNode);
			break;

		case FbxNodeAttribute::eLODGroup:
			//DisplayLodGroup(pNode);
			break;
		}
	}

	/*DisplayUserProperties(pNode);
	DisplayTarget(pNode);
	DisplayPivotsAndLimits(pNode);
	DisplayTransformPropagation(pNode);*/
	//PrintGeometricTransform(pNode);

	// Loops through all the children of this node
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		Mesh fillMesh;
		
		PrintContent(pNode->GetChild(i), &fillMesh, mats);

		mesh->children.push_back(fillMesh);
	}
}


void DisplayTarget(FbxNode* pNode)
{
    if(pNode->GetTarget() != NULL)
    {
        //DisplayString("    Target Name: ", (char *) pNode->GetTarget()->GetName());
    }
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
string PrintGeometricTransform(FbxNode* pNode)
{
    FbxVector4 lTmpVector;
	GeoTransformations transformation;
	string pString;

	ofstream binFile(BINARY_FILE, ofstream::binary | ofstream::app);


    // Translation
    lTmpVector = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
	pString += "        Translation: " + to_string(lTmpVector[0]) + " " + to_string(lTmpVector[1]) + " " + to_string(lTmpVector[2]) + "\n";
	transformation.translation[0] = (float)lTmpVector[0];
	transformation.translation[1] = (float)lTmpVector[1];
	transformation.translation[2] = (float)lTmpVector[2];

    // Rotation
    lTmpVector = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	pString += "        Rotation:    " + to_string(lTmpVector[0]) + " " + to_string(lTmpVector[1]) + " " + to_string(lTmpVector[2]) + "\n";
	transformation.rotation[0] = (float)lTmpVector[0];
	transformation.rotation[1] = (float)lTmpVector[1];
	transformation.rotation[2] = (float)lTmpVector[2];

    // Scaling
    lTmpVector = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
	pString += "        Scaling:     " + to_string(lTmpVector[0]) + " " + to_string(lTmpVector[1]) + " " + to_string(lTmpVector[2]) + "\n";
	pString += "\n";
	transformation.scale[0] = (float)lTmpVector[0];
	transformation.scale[1] = (float)lTmpVector[1];
	transformation.scale[2] = (float)lTmpVector[2];

	binFile.write((char*)&transformation, sizeof(GeoTransformations));

	binFile.close();
	return pString;
}


/*
========================================================================================================================

	DisplayMetaData has been printing empty in all the test runs of the original FBX SDK, it might not be useful to us.

	// Martina Molid

========================================================================================================================
*/
void DisplayMetaData(FbxScene* pScene)
{
    FbxDocumentInfo* sceneInfo = pScene->GetSceneInfo();
    if (sceneInfo)
    {
        FBXSDK_printf("\n\n--------------------\nMeta-Data\n--------------------\n\n");
        FBXSDK_printf("    Title: %s\n", sceneInfo->mTitle.Buffer());
        FBXSDK_printf("    Subject: %s\n", sceneInfo->mSubject.Buffer());
        FBXSDK_printf("    Author: %s\n", sceneInfo->mAuthor.Buffer());
        FBXSDK_printf("    Keywords: %s\n", sceneInfo->mKeywords.Buffer());
        FBXSDK_printf("    Revision: %s\n", sceneInfo->mRevision.Buffer());
        FBXSDK_printf("    Comment: %s\n", sceneInfo->mComment.Buffer());

        FbxThumbnail* thumbnail = sceneInfo->GetSceneThumbnail();
        if (thumbnail)
        {
            FBXSDK_printf("    Thumbnail:\n");

            switch (thumbnail->GetDataFormat())
            {
            case FbxThumbnail::eRGB_24:
                FBXSDK_printf("        Format: RGB\n");
                break;
            case FbxThumbnail::eRGBA_32:
                FBXSDK_printf("        Format: RGBA\n");
                break;
            }

            switch (thumbnail->GetSize())
            {
	        default:
	            break;
            case FbxThumbnail::eNotSet:
                FBXSDK_printf("        Size: no dimensions specified (%ld bytes)\n", thumbnail->GetSizeInBytes());
                break;
            case FbxThumbnail::e64x64:
                FBXSDK_printf("        Size: 64 x 64 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
                break;
            case FbxThumbnail::e128x128:
                FBXSDK_printf("        Size: 128 x 128 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
            }
        }
    }
}

