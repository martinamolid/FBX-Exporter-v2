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
#include "DisplaySkeleton.h"
#include "DisplayAnimation.h"

#include "Filenames.h"

#include "PrintInfo.h"
#include "PrintMaterial.h"
#include "PrintMesh.h"
#include "PrintLight.h"

using namespace std;

#pragma comment(lib,"libfbxsdk-mt.lib")
#pragma comment(lib,"libxml2-mt.lib")
#pragma comment(lib,"zlib-mt.lib")

// Local function prototypes.
void PrintContent(FbxNode* pNode, vector<Group>& fillGroup, vector<MeshHolder>& mesh, vector<PhongMaterial>& mats, vector<DirLight>& dirLight, vector<PointLight>& pointLight, bool isChild, int parentType);
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
	
	// Temporary data holders that gets parsed on file writing
	vector<MeshHolder> meshData;

	MehHeader fileHeader;
	vector<Group> groups;
	vector<Mesh> meshes;
	vector<Joint> joints;
	vector<PhongMaterial> materials;
	vector<DirLight> dirLights;
	vector<PointLight> pointLight;

	vector<Animation> animations;
	vector<KeyFrame> keyframes;
	vector<Transform> keyTransforms;

	// Gather the data
	if (sceneRootNode)
	{
		for (int i = 0; i < elementCount; i++)
		{
			PrintContent(sceneRootNode->GetChild(i), groups, meshData, materials, dirLights, pointLight, false, -1);
		}
	}

	//	===== Parse data ==================================================
	//	This section will parse and data that isn't yet fully loaded in for 
	//	the writing of the custom file
	//	===================================================================

	// ==== Header ====
	fileHeader.meshCount = (int)meshData.size();
	fileHeader.groupCount = (int)groups.size();;
	fileHeader.materialCount = (int)materials.size();
	fileHeader.dirLightCount = (int)dirLights.size();
	fileHeader.pointLightCount = (int)pointLight.size();

	// ==== Meshes ====
	for (int i = 0; i < meshData.size(); i++)
	{
		Mesh fillMesh;

		// Name
		for (int j = 0; j < NAME_SIZE; j++)
			fillMesh.name[j] = meshData[i].name[j];
		// Material
		fillMesh.materialID = meshData[i].materialID;
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

		// Skeleton data
		for (int c = 0; c < NAME_SIZE; c++)
			fillMesh.skeleton.name[c] = meshData[i].skeleton.name[c];
		fillMesh.skeleton.jointCount = (int)meshData[i].skeleton.joints.size();
		fillMesh.skeleton.aniCount = (int)meshData[i].skeleton.animations.size();

		// Joint data
		for (int j = 0; j < fillMesh.skeleton.jointCount; j++)
		{
			Joint fillJoint;
			for (int c = 0; c < NAME_SIZE; c++)
				fillJoint.name[c] = meshData[i].skeleton.joints[j].name[c];
			fillJoint.parentIndex = meshData[i].skeleton.joints[j].parentIndex;
			for (int row = 0; row < 4; row++)
				for (int col = 0; col < 4; col++)
					fillJoint.invBindPose[col + row * 4] = (float)meshData[i].skeleton.joints[j].invBindPose[row][col];

			joints.push_back(fillJoint);
		}

		// Custom attribute
		fillMesh.type = meshData[i].type;
		fillMesh.link = meshData[i].link;

		// Vertex count
		fillMesh.vertexCount = meshData[i].vertexCount;

		// Animations
		for (int a = 0; a < meshData[i].skeleton.animations.size(); a++)
		{
			Animation fillAni;
			for (int j = 0; j < NAME_SIZE; j++)
				fillAni.name[j] = meshData[i].skeleton.animations[a].name[j];
			fillAni.keyframeFirst	= meshData[i].skeleton.animations[a].keyframeFirst;
			fillAni.keyframeLast	= meshData[i].skeleton.animations[a].keyframeLast;
			fillAni.duration		= meshData[i].skeleton.animations[a].duration;
			fillAni.rate			= meshData[i].skeleton.animations[a].rate;
			fillAni.keyframeCount	= (int)meshData[i].skeleton.animations[a].keyframes.size();
			animations.push_back(fillAni);

			// Keyframes
			for (int k = 0; k < animations[a].keyframeCount; k++)
			{
				KeyFrame fillKey;
				// This expects all the transforms to be filled equally (no transform is an identity matrix
				fillKey.Transforms = (int)meshData[i].skeleton.animations[a].keyframes[k].localJointsR.size();
				for (int t = 0; t < fillKey.Transforms; t++)
				{
					Transform fillTr;
					for (int v = 0; v < 3; v++)
						fillTr.transform[v] = (float)meshData[i].skeleton.animations[a].keyframes[k].localJointsT[t][v];
					for (int v = 0; v < 4; v++)
						fillTr.rotate[v] = (float)meshData[i].skeleton.animations[a].keyframes[k].localJointsR[t][v];
					for (int v = 0; v < 3; v++)
						fillTr.scale[v] = (float)meshData[i].skeleton.animations[a].keyframes[k].localJointsS[t][v];
					keyTransforms.push_back(fillTr);
				}

				keyframes.push_back(fillKey);
			}
		}

		meshes.push_back(fillMesh);
	}



	std::cout << "Writing to ascii..." << std::endl;
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
	std::cout << "Writing header..." << std::endl;
	asciiFile2 << "//v File Header --------------------" << endl;
	asciiFile2 << "    Mesh count" << endl;
	asciiFile2 << fileHeader.meshCount << endl;					//* Binary data
	asciiFile2 << "    Mesh Group count" << endl;
	asciiFile2 << fileHeader.groupCount << endl;				//* Binary data
	asciiFile2 << "    Material count" << endl;
	asciiFile2 << fileHeader.materialCount << endl;				//* Binary data
	asciiFile2 << "    Point Light count" << endl;
	asciiFile2 << fileHeader.pointLightCount << endl;			//* Binary data
	asciiFile2 << "    Directional count" << endl;
	asciiFile2 << fileHeader.dirLightCount << endl;				//* Binary data
	asciiFile2 << "//^ File Header --------------------" << endl << endl;

	std::cout << "Writing groups..." << std::endl;
	// - 2 Groups
	for (int i = 0; i < fileHeader.groupCount; i++)
	{
		asciiFile2 << "//v Group " << i << " --------------------" << endl << endl;

		// 2.1 Group name
		asciiFile2 << "    Group name: " << endl;
		asciiFile2 << groups[i].name << endl;					//* Binary data
		// 2.2 Transformation

		asciiFile2 << "    Translation vector: " << endl;
		//v Binary data
		asciiFile2 << (float)groups[i].translation[0] << ", " << (float)groups[i].translation[1] << ", " << (float)groups[i].translation[2] << endl;

		asciiFile2 << "    Rotation vector: " << endl;
		//v Binary data
		asciiFile2 << (float)groups[i].rotation[0] << ", " << (float)groups[i].rotation[1] << ", " << (float)groups[i].rotation[2] << endl;

		asciiFile2 << "    Scale vector: " << endl;
		//v Binary data
		asciiFile2 << (float)groups[i].scale[0] << ", " << (float)groups[i].scale[1] << ", " << (float)groups[i].scale[2] << endl;

		// 2.3 Hierarchy
		asciiFile2 << "    Is child: " << endl;
		asciiFile2 << groups[i].isChild << endl;
		asciiFile2 << "    Parent Name: " << endl;
		asciiFile2 << groups[i].parentName << endl;
		asciiFile2 << "    Parent type: " << endl;
		asciiFile2 << groups[i].parentType << endl;

		asciiFile2 << "//^ Group " << i << " --------------------" << endl << endl;
		asciiFile2 << endl;
	}

	// - 3 Meshes
	std::cout << "Writing meshes..." << std::endl;
	for (int i = 0; i < fileHeader.meshCount; i++)
	{
		asciiFile2 << "//v Mesh " << i << " Header " << " --------------------" << endl;

		// 3.1 Mesh name
		asciiFile2 << "    Mesh name: " << endl;
		asciiFile2 << meshes[i].name << endl;				
		// 3.2  Material name
		asciiFile2 << "    Material ID : " << endl;		
		asciiFile2 << meshes[i].materialID << endl;			
		// 3.3 Transformation
		asciiFile2 << "    Translation vector: " << endl;
		asciiFile2 << meshData[i].translation[0] << ", " << meshData[i].translation[1] << ", " << meshData[i].translation[2] << endl;
		asciiFile2 << "    Rotation vector: " << endl;
		asciiFile2 << meshData[i].rotation[0] << ", " << meshData[i].rotation[1] << ", " << meshData[i].rotation[2] << endl;
		asciiFile2 << "    Scale vector: " << endl;
		asciiFile2 << meshData[i].scale[0] << ", " << meshData[i].scale[1] << ", " << meshData[i].scale[2] << endl; 		
		// 3.4 hierarchy
		asciiFile2 << "    Is child: " << endl;
		asciiFile2 << meshes[i].isChild << endl;
		asciiFile2 << "    Parent Name: " << endl;
		asciiFile2 << meshes[i].parentName << endl;
		asciiFile2 << "    Parent type: " << endl;
		asciiFile2 << meshes[i].parentType << endl;			
		// 3.5 Entity attributes
		asciiFile2 << "    Attribute type: " << endl;
		asciiFile2 << meshes[i].type << endl;			
		asciiFile2 << "    Attribute link: " << endl;
		asciiFile2 << meshes[i].link << endl;			
		// 3.6 Vertex count
		asciiFile2 << "    Vertex count: " << endl;
		asciiFile2 << meshes[i].vertexCount << endl;			

		// 3.7 Skeleton
		asciiFile2 << "    Joint count: " << endl;
		asciiFile2 << meshes[i].skeleton.jointCount << endl;

		// 3.8 Skeleton
		asciiFile2 << "    Animation count: " << endl;
		asciiFile2 << meshes[i].skeleton.aniCount << endl;

		asciiFile2 << "//^ Mesh " << i << " Header " <<  " --------------------" << endl << endl;
		
		// ========================================================= Vertices
		asciiFile2 << "    //v Mesh " << i << " Vertices " << " --------------------" << endl;
		for (int j = 0; j < meshes[i].vertexCount; j++)
		{
			asciiFile2 << "    ~ " << j << " Vertex position / " << "uv / " << "normal / " << "tangent / " << "binormal " << endl;
			//v Binary data
			asciiFile2 << meshData[i].vertices[j].position[0]	<< ", "	<< meshData[i].vertices[j].position[1]	<< ", "	<< meshData[i].vertices[j].position[2] << endl; 
			asciiFile2 << meshData[i].vertices[j].uv[0]			<< ", "	<< meshData[i].vertices[j].uv[1]		<< ", "	<< endl;
			asciiFile2 << meshData[i].vertices[j].normal[0]		<< ", "	<< meshData[i].vertices[j].normal[1]	<< ", "	<< meshData[i].vertices[j].normal[2] << endl;
			asciiFile2 << meshData[i].vertices[j].tangent[0]	<< ", "	<< meshData[i].vertices[j].tangent[1]	<< ", "	<< meshData[i].vertices[j].tangent[2] << endl;
			asciiFile2 << meshData[i].vertices[j].bitangent[0] << ", " << meshData[i].vertices[j].bitangent[1] << ", " << meshData[i].vertices[j].bitangent[2] << endl;
			//^ Binary data
		}
		asciiFile2 << "    //^ Mesh " << i << " Vertices " << " --------------------" << endl << endl;
		// ======================== ~


		// ========================================================= Joints		
		asciiFile2 << "    //v Mesh " << i << " Joints " << " --------------------" << endl;
		for (int j = 0; j < meshes[i].skeleton.jointCount; j++)
		{
			asciiFile2 << "    ~ " << j << " Joint name: " << endl;
			asciiFile2 << joints[j].name << endl;			
			asciiFile2 << "index: " << endl;
			asciiFile2 << joints[j].parentIndex << endl;
			asciiFile2 << "Bind pose: " << endl;
			for (int q = 0; q < 4; q++)
			{
				for (int u = 0; u < 4; u++)
					asciiFile2 << joints[j].invBindPose[u + q * 4] << " ";
				asciiFile2 << endl;
			}
			asciiFile2 << endl;
		}
		asciiFile2 << "    //^ Mesh " << i << " Joints " << " --------------------" << endl << endl;
		// ======================== ~


		// ========================================================= Animations	
		asciiFile2 << "    //v Mesh " << i << " Animations " << " --------------------" << endl;
		for (int a = 0; a < meshes[i].skeleton.aniCount; a++)
		{
			asciiFile2 << "    ~ " << a << " Animation" << endl;
			asciiFile2 << "    Animation name: " << endl;
			asciiFile2 << animations[a].name			<< endl;
			asciiFile2 << "    First keyframe: " << endl;
			asciiFile2 << animations[a].keyframeFirst	<< endl;
			asciiFile2 << "    Last keyframe: " << endl;
			asciiFile2 << animations[a].keyframeLast	<< endl;
			asciiFile2 << "    Duration: " << endl;
			asciiFile2 << animations[a].duration		<< endl;
			asciiFile2 << "    Rate: " << endl;
			asciiFile2 << animations[a].rate			<< endl;
			asciiFile2 << "    Keyframe count: " << endl;
			asciiFile2 << animations[a].keyframeCount	<< endl;
			// ======================== ~

			
			// ========================================================= Keyframes
			asciiFile2 << "    //v Animation " << a << " Keyframes " << " --------------------" << endl << endl;
			for (int k = 0; k < animations[a].keyframeCount; k++)
			{
				asciiFile2 << "    ~ " << k << " Keyframe" << endl;
				asciiFile2 << "transforms: " << endl;
				asciiFile2 << keyframes[k].Transforms << endl;
				// ========================================================= Transforms
				for (int t = 0; t < keyframes[k].Transforms; t++)
				{
					asciiFile2 << "        * " << t << " transform / rotate / scale: " << endl;
					for (int v = 0; v < 3; v++)
						asciiFile2 << keyTransforms[t].transform[v] << " ";
					asciiFile2 << endl;
					for (int v = 0; v < 4; v++)
						asciiFile2 << keyTransforms[t].rotate[v] << " ";
					asciiFile2 << endl;
					for (int v = 0; v < 3; v++)
						asciiFile2 << keyTransforms[t].scale[v] << " ";
					asciiFile2 << endl;
				}
				// ======================== ~
			}
			asciiFile2 << "    //^ Animation " << a << " Keyframes " << " --------------------" << endl << endl;
			// ======================== ~
		}
		asciiFile2 << "    //^ Mesh " << i << " Animations " << " --------------------" << endl << endl;
	}

	// - 5 Materials
	std::cout << "Writing materials..." << std::endl;
	for (int i = 0; i < fileHeader.materialCount; i++)
	{
		asciiFile2 << "// Material " << i << " --------------------" << endl;

		// 5.1 Material name
		asciiFile2 << "    Material name: " << endl;
		asciiFile2 << materials[i].name << endl;				//* Binary data

		// 5.2 Material data
		asciiFile2 << "    Ambient, diffuse, specular, emissive, opacity" << endl;
		//*v Binary data
		asciiFile2 << materials[i].ambient[0]	<< ", "	<< materials[i].ambient[1]	<< ", "	<< materials[i].ambient[2] << endl;
		asciiFile2 << materials[i].diffuse[0]	<< ", "	<< materials[i].diffuse[1]	<< ", "	<< materials[i].diffuse[2] << endl;
		asciiFile2 << materials[i].specular[0]	<< ", "	<< materials[i].specular[1]	<< ", "	<< materials[i].specular[2] << endl;
		asciiFile2 << materials[i].emissive[0]	<< ", "	<< materials[i].emissive[1]	<< ", "	<< materials[i].emissive[2] << endl;
		asciiFile2 << materials[i].opacity << endl;
		//*^ Binary data

		// 5.3 Albedo filename
		asciiFile2 << "    Albedo name: " << endl;
		asciiFile2 << materials[i].albedo << endl;				//* Binary data

		// 5.4 Normal filename
		asciiFile2 << "    Normal name: " << endl;
		asciiFile2 << materials[i].normal << endl;				//* Binary data
	}

	// - 6 Lights
	std::cout << "Writing lights..." << std::endl;
	for (int i = 0; i < fileHeader.dirLightCount; i++)
	{
		asciiFile2 << "// Directional Light " << i << " --------------------" << endl;

		// 2 Light data
		asciiFile2 << "    Position, rotation, intensity, color " << "[(float) *  10 ]" << endl;
		//*v Binary data (visual)
		asciiFile2 << dirLights[i].position[0]	<< ", " << dirLights[i].position[1] << ", " << dirLights[i].position[2] << endl;
		asciiFile2 << dirLights[i].rotation[0]	<< ", " << dirLights[i].rotation[1] << ", " << dirLights[i].rotation[2] << endl;

		asciiFile2 << dirLights[i].intensity	<< endl;
		asciiFile2 << dirLights[i].color[0]		<< ", " << dirLights[i].color[1]	<< ", " << dirLights[i].color[2] << endl;

	}

	for (int i = 0; i < fileHeader.pointLightCount; i++)
	{
		asciiFile2 << "// Point Light " << i << " --------------------" << endl;

		// 2 Light data
		asciiFile2 << "    Position, intensity, color " << "[(float) *  7 ]" << endl;
		//*v Binary data (visual)
		asciiFile2 << pointLight[i].position[0]	<< ", " << pointLight[i].position[1]	<< ", "	<< pointLight[i].position[2] << endl;

		asciiFile2 << pointLight[i].intensity	<< endl;
		asciiFile2 << pointLight[i].color[0]	<< ", " << pointLight[i].color[1]	<< ", " << pointLight[i].color[2]	<< endl;

	}
	asciiFile2.close();
	std::cout << "Ascii done!" << std::endl;
	
	std::cout << "Writing to binary..." << std::endl;
	// ===== Binary file file ==================================================
	// This is used to directly write binary data to the file
	// Binary data is as expected hard to read and isn't formated in a readableway even when turned into
	// it's relevant type when read (ex int, float, etc). It's up to the reader to know how it was formated
	// and format it in the same way upon reading.
	//	========================================================================
	std::cout << "Writing header..." << std::endl;
	ofstream binaryFile(BINARY_FILE, ofstream::binary);	// This is where out the filepath should be added comming in from the CMD
	// - 1 File Header
	binaryFile.write((char*)&fileHeader, sizeof(MehHeader));
	// - 2 Groups
	for (int i = 0; i < fileHeader.groupCount; i++)
	{
		// 1 Mesh header
		binaryFile.write((char*)&groups[i], sizeof(Group));
	}
	// - 3 Meshes
	std::cout << "Writing meshes..." << std::endl;
	for (int i = 0; i < fileHeader.meshCount; i++)
	{
		// 3.1 Mesh header
		binaryFile.write((char*)&meshes[i], sizeof(Mesh));

		// 3.2 Vertices
		binaryFile.write((char*)meshData[i].vertices, sizeof(Vertex) * meshes[i].vertexCount);

		// 3.3 Joints
		for (int j = 0; j < meshes[i].skeleton.jointCount; j++)
		{
			std::cout << "Writing joint " << j <<"..." << std::endl;
			binaryFile.write((char*)&joints[j], sizeof(Joint) * meshes[i].skeleton.jointCount);
		}

		for (int a = 0; a < meshes[i].skeleton.aniCount; a++)
		{
			// 3.4.1 Animations
			std::cout << "Writing animation " << a << "..." << std::endl;
			binaryFile.write((char*)&animations[a], sizeof(Joint));
			for (int k = 0; k < animations[a].keyframeCount; k++)
			{
				// 3.4.2 Keyframes
				std::cout << "Writing keyframe " << k << "..." << std::endl;
				binaryFile.write((char*)&keyframes[k], sizeof(KeyFrame));
				for (int t = 0; t < keyframes[k].Transforms; t++)
				{
					// 3.4.3 Transforms
					std::cout << "Writing keyTransform " << t << "..." << std::endl;
					binaryFile.write((char*)&keyTransforms[t], sizeof(Transform));
				}
			}
		}
	}

	// - 4 Materials
	for (int i = 0; i < fileHeader.materialCount; i++)
	{
		binaryFile.write((char*)&materials[i], sizeof(PhongMaterial));
	}

	// - 4 Light
	std::cout << "Writing lights..." << std::endl;
	// *Add light binary writing (1 forloop for each type, copy this one for more light types)
	// Change meshCount to lights or kaputt
	for (int i = 0; i < fileHeader.dirLightCount; i++)
	{
		binaryFile.write((char*)&dirLights[i], sizeof(DirLight));
	}

	for (int i = 0; i < fileHeader.pointLightCount; i++)
	{
		binaryFile.write((char*)&pointLight[i], sizeof(PointLight));
	}
	binaryFile.close();
	std::cout << "Binary done!" << std::endl;



    // Destroy all objects created by the FBX SDK.
    DestroySdkObjects(lSdkManager, lResult);
	system("pause");
    return 0;
}

/*========================================================================================================================
	PrintContent recursively prints all information in a node (and its children), determined by the type of the node.
========================================================================================================================*/
void PrintContent(FbxNode* pNode, vector<Group>& groups, vector<MeshHolder>& meshes, vector<PhongMaterial>& mats, vector<DirLight>& dirLight, vector<PointLight>& pointLight, bool isChild, int parentType)
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


		case FbxNodeAttribute::eLight:

			DirLight fillDirLight;
			PointLight fillPointLight;
			{
				int type = PrintLight(pNode, &fillDirLight, &fillPointLight);
				if (type == 1)
				{
					for (int i = 0; i < 3; i++)
					{
						fillDirLight.position[i] = (float)translation[i];
						fillDirLight.rotation[i] = (float)rotation[i];

					}

					dirLight.push_back(fillDirLight);
				}
				else if (type == 2)
				{
					for (int i = 0; i < 3; i++)
						fillPointLight.position[i] = (float)translation[i];

					pointLight.push_back(fillPointLight);
				}
			}
			break;

		}
	}

	// Loops through all the children of this node
	for (int i = 0; i < pNode->GetChildCount(); i++)
	{
		PrintContent(pNode->GetChild(i), groups, meshes, mats, dirLight, pointLight, true, parentType);
		//PrintContent(pNode->GetChild(i), &fillMesh, &fillDirLight, &fillSpotLight, mats);
	}
}

