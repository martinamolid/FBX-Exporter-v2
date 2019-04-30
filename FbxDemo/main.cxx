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

#include <iostream>
#include <fstream>
using namespace std;

#pragma comment(lib,"libfbxsdk-mt.lib")
#pragma comment(lib,"libxml2-mt.lib")
#pragma comment(lib,"zlib-mt.lib")

// Local function prototypes.
string PrintContent(FbxScene* pScene);
string PrintContent(FbxNode* pNode);
void DisplayTarget(FbxNode* pNode);
void DisplayTransformPropagation(FbxNode* pNode);
string PrintGeometricTransform(FbxNode* pNode);
void DisplayMetaData(FbxScene* pScene);

struct GeoTransformations {
	float translation[3];
	float rotation[3];
	float scale[3];
};

static bool gVerbose = true;

int main(int argc, char** argv)
{
    FbxManager* lSdkManager = NULL;
    FbxScene* lScene = NULL;
    bool lResult;

    // Prepare the FBX SDK.
    InitializeSdkObjects(lSdkManager, lScene);
    // Load the scene.
	
	// MM: This opens the .fbx file to be read from
	FbxString lFilePath("");

	if( lFilePath.IsEmpty() )
	{
		lFilePath = IN_FBX_FILEPATH.c_str();
		lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer());
        //lResult = false;
	}

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

		if (gVerbose) asciiFile << PrintNrOfMeshes(lScene);//PrintHierarchy(lScene); // MM: Prints how many meshes exists in the FBX file

        //FBXSDK_printf("\n\n------------\nNode Content\n------------\n\n");

        if( gVerbose ) asciiFile << PrintContent(lScene);	// MM: Prints node content, currently only meshes ( see below )

        //FBXSDK_printf("\n\n----\nPose\n----\n\n");

        //if( gVerbose ) //DisplayPose(lScene);

        //FBXSDK_printf("\n\n---------\nAnimation\n---------\n\n");

        //if( gVerbose ) //DisplayAnimation(lScene);

        //now display generic information

        //FBXSDK_printf("\n\n---------\nGeneric Information\n---------\n\n");
       // if( gVerbose ) {//DisplayGenericInfo(lScene);
		//}

    }

    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);

	asciiFile.close();
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
    int i;
    FbxNode* lNode = pScene->GetRootNode();

	string pString;

    if(lNode)
    {
        for(i = 0; i < lNode->GetChildCount(); i++)
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
    //pString += PrintGeometricTransform(pNode);

    for(i = 0; i < pNode->GetChildCount(); i++)
    {
        pString += PrintContent(pNode->GetChild(i));
		
    }

	return pString;
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
	transformation.translation[0] = lTmpVector[0];
	transformation.translation[1] = lTmpVector[1];
	transformation.translation[2] = lTmpVector[2];

    // Rotation
    lTmpVector = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
	pString += "        Rotation:    " + to_string(lTmpVector[0]) + " " + to_string(lTmpVector[1]) + " " + to_string(lTmpVector[2]) + "\n";
	transformation.rotation[0] = lTmpVector[0];
	transformation.rotation[1] = lTmpVector[1];
	transformation.rotation[2] = lTmpVector[2];

    // Scaling
    lTmpVector = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
	pString += "        Scaling:     " + to_string(lTmpVector[0]) + " " + to_string(lTmpVector[1]) + " " + to_string(lTmpVector[2]) + "\n";
	pString += "\n";
	transformation.scale[0] = lTmpVector[0];
	transformation.scale[1] = lTmpVector[1];
	transformation.scale[2] = lTmpVector[2];

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

