#include "PrintMesh.h"
#include "DisplayCommon.h"

//#define MAT_HEADER_LENGTH 200

/*
========================================================================================================================

	PrintMesh calls all the functions to print the mesh's information, right now it gets the mesh's name (which it sends to PrintPolygons), 
		as well as calls PrintPolygons and PrintMaterial.

	PrintPolygons prints the mesh's information, including mesh name, nr of materials, nr of vertices and vertex data (position, uv, normals, tangents and binormals),
		and calls PrintMaterials to print materials and textures.

	PrintControlPoints works as a testing ground, as it's not used any longer. (It prints the vertices positions, while PrintPolygons builds the faces and prints)

	Returning the pString, or the ASCII information string from these functions is important, as the information string will be collected back into main
		where it will be printed to the ASCII file. (This only requires the ASCII file to be opened once)
		This is why most Print-- versions of the functions return strings.


	Some unused code from the original DisplayMesh function is left here as a reference, in case it will be used later in the project.

	** PLEASE DO NOT REMOVE OUTCOMMENTED CODE UNLESS YOU ARE SURE IT WILL NOT BE USED LATER **

	// Martina Molid

========================================================================================================================
*/

// MM: The outcommented functions are not used right now, but may be needed at a later point.
//string PrintControlsPoints(FbxMesh* pMesh);
string PrintPolygons(FbxMesh* pMesh, string meshName);
void GetPolygons(FbxMesh* pMesh, MeshHolder* mesh);
//string PrintMaterialMapping(FbxMesh* pMesh);
//void PrintTextureMapping(FbxMesh* pMesh);
//void PrintTextureNames(FbxProperty &pProperty, FbxString& pConnectionString);
//void PrintMaterialConnections(FbxMesh* pMesh);
//void PrintMaterialTextureConnections(FbxSurfaceMaterial* pMaterial, char * header, int pMatId, int l);

string PrintMesh(FbxNode* pNode) 
{
	FbxMesh* pMesh = (FbxMesh*)pNode->GetNodeAttribute();
	string pString;

	string name = (char *)pNode->GetName();		// MM: We have to get the mesh name here, then we pass it to PrintPolygons to be printed there instead of opening the binFile here too.

	///pString += PrintControlsPoints(pMesh);
	pString += PrintPolygons(pMesh, name);
	pString += "\n";
	pString += PrintMaterial(pMesh);		// MM: Textures are called to be printed from PrintMaterial
	pString += "\n";

	return pString;
}

//string PrintControlsPoints(FbxMesh* pMesh)
//{
//	string pString;
//	int i, lControlPointsCount = pMesh->GetControlPointsCount();
//	FbxVector4* lControlPoints = pMesh->GetControlPoints();
//
//	//ofstream binFile ("xBin.bin", ofstream::binary);
//
//	struct vtxCoord {
//		float x, y, z;
//	};
//
//	int size = 0;
//
//	//vtxCoord *farr = nullptr;
//
//	pString += "        Control Points Count: " + to_string(lControlPointsCount) + "\n";
//	// DEFINE SIZE HERE BASED ON CONTROL POINTS
//	//size = sizeof(vtxCoord) * lControlPointsCount;
//	//farr = new vtxCoord[size];
//
//	//binFile.write((char*)&lControlPointsCount, sizeof(int));
//
//	for (i = 0; i < lControlPointsCount; i++)
//	{
//		//pString += "        Control Point " + to_string(i);
//		pString += Print3DVector("            Coordinates: ", lControlPoints[i]);
//		//pString += Print3DVector("", lControlPoints[i]);
//
//		//farr[i].x = lControlPoints[i][0];
//		//farr[i].y = lControlPoints[i][1];
//		//farr[i].z = lControlPoints[i][2];
//
//		for (int j = 0; j < pMesh->GetElementNormalCount(); j++)
//		{
//			FbxGeometryElementNormal* leNormals = pMesh->GetElementNormal(j);
//			if (leNormals->GetMappingMode() == FbxGeometryElement::eByControlPoint)
//			{
//				char header[100];
//				FBXSDK_sprintf(header, 100, "            Normal Vector: ");
//				if (leNormals->GetReferenceMode() == FbxGeometryElement::eDirect)
//					pString += Print3DVector(header, leNormals->GetDirectArray().GetAt(i));
//			}
//		}
//	}
//
//	//binFile.write((char*) farr, size);
//	//binFile.write((char*)&f, sizeof(float));
//	//binFile.close();
//	//delete[] farr;
//
//	return pString;
//}

string PrintPolygons(FbxMesh* pMesh, string meshName)
{
	int i, j, lPolygonCount = pMesh->GetPolygonCount();
	FbxVector4* lControlPoints = pMesh->GetControlPoints();
	char header[100];

	// MM: Defines pString to print ASCII info into, and opens the binary file in binary append mode
	string pString;
	ofstream binFile(BINARY_FILE, ofstream::binary | ofstream::app);

	// --- MM: Getting, formatting and printing mesh name --- 
	int nameLength = (int)strlen(meshName.c_str());
	char finalMeshName[NAME_SIZE];
	for (int i = 0; i < nameLength; i++) {
		finalMeshName[i] = meshName[i];
	}
	finalMeshName[nameLength] = '\0'; // Puts a \0 at the end of the mesh name, still printing out whitespace into the binary file
	
	cout << "Mesh name: " << finalMeshName << endl;
	pString += "Mesh name: " + meshName + "\n";
	binFile.write((char*)finalMeshName, sizeof(char) * NAME_SIZE);

	// --- MM: Printing Material Count for the mesh ---
	int nrOfMat = pMesh->GetElementMaterialCount();
	//cout << "Material Count in PrintPolygons: " << nrOfMat << endl;
	pString += "mat count " + to_string(nrOfMat) + "\n";
	binFile.write((char*)&nrOfMat, sizeof(int));

	// --- MM: Getting and priting Vertex Count for the mesh ---
	int vtxCount = pMesh->GetPolygonVertexCount();
	pString += "vtx count " + to_string(vtxCount) + "\n";
	binFile.write((char*)&vtxCount, sizeof(int));

	// --- MM: Allocating memory for the vertices array based on the vertex count ---
	Vertex *vertices = new Vertex[vtxCount];


	// MM: Builds vertices for each polygon, and adds to the vertices array we allocated memory for
	int vertexId = 0;
	for (i = 0; i < lPolygonCount; i++)
	{
		int l;
		for (l = 0; l < pMesh->GetElementPolygonGroupCount(); l++)
		{
			FbxGeometryElementPolygonGroup* lePolgrp = pMesh->GetElementPolygonGroup(l);
			switch (lePolgrp->GetMappingMode())
			{
			case FbxGeometryElement::eByPolygon:
				if (lePolgrp->GetReferenceMode() == FbxGeometryElement::eIndex)
				{
					FBXSDK_sprintf(header, 100, "        Assigned to group: ");
					int polyGroupId = lePolgrp->GetIndexArray().GetAt(i);
					pString += header + to_string(polyGroupId) + "\n";
					break;
				}
			default:
				// any other mapping modes don't make sense
				pString += "        \"unsupported group assignment\"\n";
				break;
			}
		}

		int lPolygonSize = pMesh->GetPolygonSize(i);

		for (j = 0; j < lPolygonSize; j++)
		{
			int lControlPointIndex = pMesh->GetPolygonVertex(i, j);
			if (lControlPointIndex < 0)
			{
				//DisplayString("            Coordinates: Invalid index found!");
				continue;
			}
			else
			{
				// MM: Prints and adds -vertex positions- to the pString and to the vertices array
				pString += Print3DVector("v ", lControlPoints[lControlPointIndex]);
				vertices[vertexId].position[0] = (float)lControlPoints[lControlPointIndex][0];
				vertices[vertexId].position[1] = (float)lControlPoints[lControlPointIndex][1];
				vertices[vertexId].position[2] = (float)lControlPoints[lControlPointIndex][2];

			}

			for (l = 0; l < pMesh->GetElementVertexColorCount(); l++)
			{
				FbxGeometryElementVertexColor* leVtxc = pMesh->GetElementVertexColor(l);
				FBXSDK_sprintf(header, 100, "            Color vertex: ");

				switch (leVtxc->GetMappingMode())
				{
				default:
					break;
				case FbxGeometryElement::eByControlPoint:
					switch (leVtxc->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						pString += PrintColor(header, leVtxc->GetDirectArray().GetAt(lControlPointIndex));
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leVtxc->GetIndexArray().GetAt(lControlPointIndex);
						pString += PrintColor(header, leVtxc->GetDirectArray().GetAt(id));
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
					break;

				case FbxGeometryElement::eByPolygonVertex:
				{
					switch (leVtxc->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						pString += PrintColor(header, leVtxc->GetDirectArray().GetAt(vertexId));
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leVtxc->GetIndexArray().GetAt(vertexId);
						pString += PrintColor(header, leVtxc->GetDirectArray().GetAt(id));
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
				}
				break;

				case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
				case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
				case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
					break;
				}
			}
			for (l = 0; l < pMesh->GetElementUVCount(); ++l)
			{
				FbxGeometryElementUV* leUV = pMesh->GetElementUV(l);
				FBXSDK_sprintf(header, 100, "vt ");
				// MM: Prints and adds -UV positions- to the pString and to the vertices array
				switch (leUV->GetMappingMode())
				{
				default:
					break;
				case FbxGeometryElement::eByControlPoint:
					switch (leUV->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						pString += Print2DVector(header, leUV->GetDirectArray().GetAt(lControlPointIndex));
						vertices[vertexId].uv[0] = (float)leUV->GetDirectArray().GetAt(lControlPointIndex)[0];
						vertices[vertexId].uv[1] = (float)leUV->GetDirectArray().GetAt(lControlPointIndex)[1];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = (int)leUV->GetIndexArray().GetAt(lControlPointIndex);
						pString += Print2DVector(header, leUV->GetDirectArray().GetAt(id));
						vertices[vertexId].uv[0] = (float)leUV->GetDirectArray().GetAt(id)[0];
						vertices[vertexId].uv[1] = (float)leUV->GetDirectArray().GetAt(id)[1];
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
					break;

				case FbxGeometryElement::eByPolygonVertex:
				{
					int lTextureUVIndex = pMesh->GetTextureUVIndex(i, j);
					switch (leUV->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
					case FbxGeometryElement::eIndexToDirect:
					{
						pString += Print2DVector(header, leUV->GetDirectArray().GetAt(lTextureUVIndex));
						vertices[vertexId].uv[0] = (float)leUV->GetDirectArray().GetAt(lTextureUVIndex)[0];
						vertices[vertexId].uv[1] = (float)leUV->GetDirectArray().GetAt(lTextureUVIndex)[1];
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
				}
				break;

				case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
				case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
				case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
					break;
				}
			}
			for (l = 0; l < pMesh->GetElementNormalCount(); ++l)
			{
				FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(l);
				FBXSDK_sprintf(header, 100, "vn ");
				// MM: Prints and adds -normal positions- to the pString and to the vertices array
				if (leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leNormal->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						pString += Print3DVector(header, leNormal->GetDirectArray().GetAt(vertexId));
						vertices[vertexId].normal[0] = (float)leNormal->GetDirectArray().GetAt(vertexId)[0];
						vertices[vertexId].normal[1] = (float)leNormal->GetDirectArray().GetAt(vertexId)[1];
						vertices[vertexId].normal[2] = (float)leNormal->GetDirectArray().GetAt(vertexId)[2];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leNormal->GetIndexArray().GetAt(vertexId);
						pString += Print3DVector(header, leNormal->GetDirectArray().GetAt(id));
						vertices[vertexId].normal[0] = (float)leNormal->GetDirectArray().GetAt(id)[0];
						vertices[vertexId].normal[1] = (float)leNormal->GetDirectArray().GetAt(id)[1];
						vertices[vertexId].normal[2] = (float)leNormal->GetDirectArray().GetAt(id)[2];
					}
					break;
					default:
						break; // other reference modes not shown here!
					}

				}
				// MM: This one is used for spheres and planes, as they previously did not output normals
				if (leNormal->GetMappingMode() == FbxGeometryElement::eByControlPoint)
				{
					if (leNormal->GetReferenceMode() == FbxGeometryElement::eDirect) {
						pString += Print3DVector(header, leNormal->GetDirectArray().GetAt(i));
						vertices[vertexId].normal[0] = (float)leNormal->GetDirectArray().GetAt(vertexId)[0];
						vertices[vertexId].normal[1] = (float)leNormal->GetDirectArray().GetAt(vertexId)[1];
						vertices[vertexId].normal[2] = (float)leNormal->GetDirectArray().GetAt(vertexId)[2];
					}
				}

			}
			for (l = 0; l < pMesh->GetElementTangentCount(); ++l)
			{
				FbxGeometryElementTangent* leTangent = pMesh->GetElementTangent(l);
				FBXSDK_sprintf(header, 100, "vtan ");
				// MM: Prints and adds -tangents- to the pString and to the vertices array
				if (leTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leTangent->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						pString += Print3DVector(header, leTangent->GetDirectArray().GetAt(vertexId));
						vertices[vertexId].tangent[0] = (float)leTangent->GetDirectArray().GetAt(vertexId)[0];
						vertices[vertexId].tangent[1] = (float)leTangent->GetDirectArray().GetAt(vertexId)[1];
						vertices[vertexId].tangent[2] = (float)leTangent->GetDirectArray().GetAt(vertexId)[2];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leTangent->GetIndexArray().GetAt(vertexId);
						pString += Print3DVector(header, leTangent->GetDirectArray().GetAt(id));
						vertices[vertexId].tangent[0] = (float)leTangent->GetDirectArray().GetAt(id)[0];
						vertices[vertexId].tangent[1] = (float)leTangent->GetDirectArray().GetAt(id)[1];
						vertices[vertexId].tangent[2] = (float)leTangent->GetDirectArray().GetAt(id)[2];
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
				}

			}
			for (l = 0; l < pMesh->GetElementBinormalCount(); ++l)
			{

				FbxGeometryElementBinormal* leBinormal = pMesh->GetElementBinormal(l);
				FBXSDK_sprintf(header, 100, "vbin ");
				// MM: Prints and adds -binormals- to the pString and to the vertices array
				if (leBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leBinormal->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						pString += Print3DVector(header, leBinormal->GetDirectArray().GetAt(vertexId));
						vertices[vertexId].bitangent[0] = (float)leBinormal->GetDirectArray().GetAt(vertexId)[0];
						vertices[vertexId].bitangent[1] = (float)leBinormal->GetDirectArray().GetAt(vertexId)[1];
						vertices[vertexId].bitangent[2] = (float)leBinormal->GetDirectArray().GetAt(vertexId)[2];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leBinormal->GetIndexArray().GetAt(vertexId);
						pString += Print3DVector(header, leBinormal->GetDirectArray().GetAt(id));
						vertices[vertexId].bitangent[0] = (float)leBinormal->GetDirectArray().GetAt(id)[0];
						vertices[vertexId].bitangent[1] = (float)leBinormal->GetDirectArray().GetAt(id)[1];
						vertices[vertexId].bitangent[2] = (float)leBinormal->GetDirectArray().GetAt(id)[2];
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
				}
			}
			vertexId++;
		} // for polygonSize
	} // for polygonCount

	// MM: Write the vertices to the binary file
	binFile.write((char*)vertices, sizeof(Vertex)*vtxCount);
	// MM: Delete the allocated memory for vertices
	delete[] vertices;
	// MM: Close the binary file
	binFile.close();
	return pString;
}

void GetMesh(FbxNode* pNode, MeshHolder* mesh, vector<PhongMaterial2>& materials)
{
	FbxMesh* fbxMesh = (FbxMesh*)pNode->GetNodeAttribute();

	// Applies the mesh name
	int nameLength = (int)strlen(pNode->GetName());
	string nameBuffer = pNode->GetName();
	for (int j = 0; j < nameLength; j++)
	{
		mesh->name[j] = nameBuffer[j];
	}
	mesh->name[nameLength] = '\0';
	// Puts a \0 at the end of the mesh name, still printing out whitespace into the binary file


	//pString += PrintControlsPoints(pMesh);
	GetPolygons(fbxMesh, mesh);
	PrintMaterial(fbxMesh, materials, mesh);	// MM: Textures are called to be printed from PrintMaterial
	DisplayUserProperties(pNode, mesh);
	
}

void GetPolygons(FbxMesh* fbxMesh, MeshHolder* mesh)
{
	int i, j, lPolygonCount = fbxMesh->GetPolygonCount();
	FbxVector4* lControlPoints = fbxMesh->GetControlPoints();
	char header[100];

	// --- MM: Allocating memory for the vertices array based on the vertex count ---
	int vtxCount = fbxMesh->GetPolygonVertexCount();
	Vertex *vertices = new Vertex[vtxCount];
	mesh->vertices = new Vertex[vtxCount];

	// MM: Builds vertices for each polygon, and adds to the vertices array we allocated memory for
	int vertexId = 0;
	for (i = 0; i < lPolygonCount; i++)
	{
		int l;
		for (l = 0; l < fbxMesh->GetElementPolygonGroupCount(); l++)
		{
			FbxGeometryElementPolygonGroup* lePolgrp = fbxMesh->GetElementPolygonGroup(l);
			switch (lePolgrp->GetMappingMode())
			{
			case FbxGeometryElement::eByPolygon:
				if (lePolgrp->GetReferenceMode() == FbxGeometryElement::eIndex)
				{
					FBXSDK_sprintf(header, 100, "        Assigned to group: ");
					int polyGroupId = lePolgrp->GetIndexArray().GetAt(i);
					break;
				}
			default:
				// any other mapping modes don't make sense
				//\"unsupported group assignment\"\n";
				break;
			}
		}

		int lPolygonSize = fbxMesh->GetPolygonSize(i);

		for (j = 0; j < lPolygonSize; j++)
		{
			int lControlPointIndex = fbxMesh->GetPolygonVertex(i, j);
			if (lControlPointIndex < 0)
			{
				//DisplayString("            Coordinates: Invalid index found!");
				continue;
			}
			else
			{
				// MM: Prints and adds -vertex positions- to the pString and to the vertices array
				Print3DVector("v ", lControlPoints[lControlPointIndex]);
				vertices[vertexId].position[0] = (float)lControlPoints[lControlPointIndex][0];
				vertices[vertexId].position[1] = (float)lControlPoints[lControlPointIndex][1];
				vertices[vertexId].position[2] = (float)lControlPoints[lControlPointIndex][2];

			}

			for (l = 0; l < fbxMesh->GetElementVertexColorCount(); l++)
			{
				FbxGeometryElementVertexColor* leVtxc = fbxMesh->GetElementVertexColor(l);
				FBXSDK_sprintf(header, 100, "            Color vertex: ");

				switch (leVtxc->GetMappingMode())
				{
				default:
					break;
				case FbxGeometryElement::eByControlPoint:
					switch (leVtxc->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						PrintColor(header, leVtxc->GetDirectArray().GetAt(lControlPointIndex));
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leVtxc->GetIndexArray().GetAt(lControlPointIndex);
						PrintColor(header, leVtxc->GetDirectArray().GetAt(id));
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
					break;

				case FbxGeometryElement::eByPolygonVertex:
				{
					switch (leVtxc->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						PrintColor(header, leVtxc->GetDirectArray().GetAt(vertexId));
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leVtxc->GetIndexArray().GetAt(vertexId);
						PrintColor(header, leVtxc->GetDirectArray().GetAt(id));
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
				}
				break;

				case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
				case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
				case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
					break;
				}
			}
			for (l = 0; l < fbxMesh->GetElementUVCount(); ++l)
			{
				FbxGeometryElementUV* leUV = fbxMesh->GetElementUV(l);
				FBXSDK_sprintf(header, 100, "vt ");
				// MM: Prints and adds -UV positions- to the pString and to the vertices array
				switch (leUV->GetMappingMode())
				{
				default:
					break;
				case FbxGeometryElement::eByControlPoint:
					switch (leUV->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						Print2DVector(header, leUV->GetDirectArray().GetAt(lControlPointIndex));
						vertices[vertexId].uv[0] = (float)leUV->GetDirectArray().GetAt(lControlPointIndex)[0];
						vertices[vertexId].uv[1] = (float)leUV->GetDirectArray().GetAt(lControlPointIndex)[1];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leUV->GetIndexArray().GetAt(lControlPointIndex);
						Print2DVector(header, leUV->GetDirectArray().GetAt(id));
						vertices[vertexId].uv[0] = (float)leUV->GetDirectArray().GetAt(id)[0];
						vertices[vertexId].uv[1] = (float)leUV->GetDirectArray().GetAt(id)[1];
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
					break;

				case FbxGeometryElement::eByPolygonVertex:
				{
					int lTextureUVIndex = fbxMesh->GetTextureUVIndex(i, j);
					switch (leUV->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
					case FbxGeometryElement::eIndexToDirect:
					{
						Print2DVector(header, leUV->GetDirectArray().GetAt(lTextureUVIndex));
						vertices[vertexId].uv[0] = (float)leUV->GetDirectArray().GetAt(lTextureUVIndex)[0];
						vertices[vertexId].uv[1] = (float)leUV->GetDirectArray().GetAt(lTextureUVIndex)[1];
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
				}
				break;

				case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
				case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
				case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
					break;
				}
			}
			for (l = 0; l < fbxMesh->GetElementNormalCount(); ++l)
			{
				FbxGeometryElementNormal* leNormal = fbxMesh->GetElementNormal(l);
				FBXSDK_sprintf(header, 100, "vn ");
				// MM: Prints and adds -normal positions- to the pString and to the vertices array
				if (leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leNormal->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						Print3DVector(header, leNormal->GetDirectArray().GetAt(vertexId));
						vertices[vertexId].normal[0] = (float)leNormal->GetDirectArray().GetAt(vertexId)[0];
						vertices[vertexId].normal[1] = (float)leNormal->GetDirectArray().GetAt(vertexId)[1];
						vertices[vertexId].normal[2] = (float)leNormal->GetDirectArray().GetAt(vertexId)[2];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leNormal->GetIndexArray().GetAt(vertexId);
						Print3DVector(header, leNormal->GetDirectArray().GetAt(id));
						vertices[vertexId].normal[0] = (float)leNormal->GetDirectArray().GetAt(id)[0];
						vertices[vertexId].normal[1] = (float)leNormal->GetDirectArray().GetAt(id)[1];
						vertices[vertexId].normal[2] = (float)leNormal->GetDirectArray().GetAt(id)[2];
					}
					break;
					default:
						break; // other reference modes not shown here!
					}

				}
				// MM: This one is used for spheres and planes, as they previously did not output normals
				if (leNormal->GetMappingMode() == FbxGeometryElement::eByControlPoint)
				{
					if (leNormal->GetReferenceMode() == FbxGeometryElement::eDirect) {
						Print3DVector(header, leNormal->GetDirectArray().GetAt(i));
						vertices[vertexId].normal[0] = (float)leNormal->GetDirectArray().GetAt(vertexId)[0];
						vertices[vertexId].normal[1] = (float)leNormal->GetDirectArray().GetAt(vertexId)[1];
						vertices[vertexId].normal[2] = (float)leNormal->GetDirectArray().GetAt(vertexId)[2];
					}
				}

			}
			for (l = 0; l < fbxMesh->GetElementTangentCount(); ++l)
			{
				FbxGeometryElementTangent* leTangent = fbxMesh->GetElementTangent(l);
				FBXSDK_sprintf(header, 100, "vtan ");
				// MM: Prints and adds -tangents- to the pString and to the vertices array
				if (leTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leTangent->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						Print3DVector(header, leTangent->GetDirectArray().GetAt(vertexId));
						vertices[vertexId].tangent[0] = (float)leTangent->GetDirectArray().GetAt(vertexId)[0];
						vertices[vertexId].tangent[1] = (float)leTangent->GetDirectArray().GetAt(vertexId)[1];
						vertices[vertexId].tangent[2] = (float)leTangent->GetDirectArray().GetAt(vertexId)[2];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leTangent->GetIndexArray().GetAt(vertexId);
						Print3DVector(header, leTangent->GetDirectArray().GetAt(id));
						vertices[vertexId].tangent[0] = (float)leTangent->GetDirectArray().GetAt(id)[0];
						vertices[vertexId].tangent[1] = (float)leTangent->GetDirectArray().GetAt(id)[1];
						vertices[vertexId].tangent[2] = (float)leTangent->GetDirectArray().GetAt(id)[2];
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
				}

			}
			for (l = 0; l < fbxMesh->GetElementBinormalCount(); ++l)
			{

				FbxGeometryElementBinormal* leBinormal = fbxMesh->GetElementBinormal(l);
				FBXSDK_sprintf(header, 100, "vbin ");
				// MM: Prints and adds -binormals- to the pString and to the vertices array
				if (leBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
				{
					switch (leBinormal->GetReferenceMode())
					{
					case FbxGeometryElement::eDirect:
						Print3DVector(header, leBinormal->GetDirectArray().GetAt(vertexId));
						vertices[vertexId].bitangent[0] = (float)leBinormal->GetDirectArray().GetAt(vertexId)[0];
						vertices[vertexId].bitangent[1] = (float)leBinormal->GetDirectArray().GetAt(vertexId)[1];
						vertices[vertexId].bitangent[2] = (float)leBinormal->GetDirectArray().GetAt(vertexId)[2];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leBinormal->GetIndexArray().GetAt(vertexId);
						Print3DVector(header, leBinormal->GetDirectArray().GetAt(id));
						vertices[vertexId].bitangent[0] = (float)leBinormal->GetDirectArray().GetAt(id)[0];
						vertices[vertexId].bitangent[1] = (float)leBinormal->GetDirectArray().GetAt(id)[1];
						vertices[vertexId].bitangent[2] = (float)leBinormal->GetDirectArray().GetAt(id)[2];
					}
					break;
					default:
						break; // other reference modes not shown here!
					}
				}
			}
			vertexId++;
		} // for polygonSize
	} // for polygonCount

	// Copies the pointerdata created here into the pointer in the recived mesh struct
	// Mesh should be arriving from a vector
	// memcpy - the vertices struct containts 14 floats, additionally there are vtxCount amount of vertices
	// This is the amount of memory that will be copied into the mesh pointer
	mesh->vertexCount = vtxCount;
	memcpy(mesh->vertices, vertices, sizeof(float) * 14 * vtxCount);


	// MM: Delete the allocated memory for vertices
	if (vertices)
		delete[] vertices;
}

/*
========================================================================================================================
This function loops through an FbxObjects properties. Which is the custom attributes added in Maya.

First setting "Type" and "Link" in the Meshholder to 0;
Loops through each property.
If the property is an int, (eFbxInt)
then it checks if the name of the property is "Type" or "Link"
Then retrives the information and stores it.
========================================================================================================================
*/
void DisplayUserProperties(FbxObject* pObject, MeshHolder* mesh)
{
	int lCount = 0;
	FbxString lTitleStr = "    Property Count: ";

	FbxProperty lProperty = pObject->GetFirstProperty();
	while (lProperty.IsValid())
	{
		if (lProperty.GetFlag(FbxPropertyFlags::eUserDefined))
			lCount++;

		lProperty = pObject->GetNextProperty(lProperty);
	}

	if (lCount == 0)
		return; // there are no user properties to display

	DisplayInt(lTitleStr.Buffer(), lCount);

	mesh->type = 0;
	mesh->link = 0;

	lProperty = pObject->GetFirstProperty();
	int i = 0;
	while (lProperty.IsValid())
	{
		if (lProperty.GetFlag(FbxPropertyFlags::eUserDefined))
		{
			DisplayInt("        Property ", i);
			FbxString lString = lProperty.GetLabel();
			DisplayString("            Display Name: ", lString.Buffer());
			lString = lProperty.GetName();
			DisplayString("            Internal Name: ", lString.Buffer());
			DisplayString("            Type: ", lProperty.GetPropertyDataType().GetName());
			if (lProperty.HasMinLimit()) DisplayDouble("            Min Limit: ", lProperty.GetMinLimit());
			if (lProperty.HasMaxLimit()) DisplayDouble("            Max Limit: ", lProperty.GetMaxLimit());
			DisplayBool("            Is Animatable: ", lProperty.GetFlag(FbxPropertyFlags::eAnimatable));

			FbxDataType lPropertyDataType = lProperty.GetPropertyDataType();

			// INTEGER
			if (lPropertyDataType.GetType() == eFbxInt)
			{
				DisplayInt("            Default Value: ", lProperty.Get<FbxInt>());

				if (lProperty.GetName() == "Type")
				{
					DisplayString("            Found: Type ");
					mesh->type = lProperty.Get<FbxInt>();
				}

				if (lProperty.GetName() == "Link")
				{
					DisplayString("            Found: Link ");
					mesh->link = lProperty.Get<FbxInt>();
				}
			}
			// UNIDENTIFIED
			else
			{
				DisplayString("            Default Value: UNIDENTIFIED");
			}
			i++;
		}

		lProperty = pObject->GetNextProperty(lProperty);
	}
}

//string PrintMaterialMapping(FbxMesh* pMesh)
//{
//	const char* lMappingTypes[] = { "None", "By Control Point", "By Polygon Vertex", "By Polygon", "By Edge", "All Same" };
//	const char* lReferenceMode[] = { "Direct", "Index", "Index to Direct" };
//
//	string pString; 
//
//	int lMtrlCount = 0;
//	FbxNode* lNode = NULL;
//	if (pMesh) {
//		lNode = pMesh->GetNode();
//		if (lNode)
//			lMtrlCount = lNode->GetMaterialCount();
//	}
//
//	for (int l = 0; l < pMesh->GetElementMaterialCount(); l++)
//	{
//		FbxGeometryElementMaterial* leMat = pMesh->GetElementMaterial(l);
//		if (leMat)
//		{
//			char header[100];
//			FBXSDK_sprintf(header, 100, "    Material Element %d: ", l);
//			pString += PrintString(header);
//			//pString += header;
//
//
//			pString += PrintString("           Mapping: ", lMappingTypes[leMat->GetMappingMode()]);
//			pString += PrintString("           ReferenceMode: ", lReferenceMode[leMat->GetReferenceMode()]);
//
//			int lMaterialCount = 0;
//			//FbxString lString;
//
//			if (leMat->GetReferenceMode() == FbxGeometryElement::eDirect ||
//				leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
//			{
//				lMaterialCount = lMtrlCount;
//			}
//
//			if (leMat->GetReferenceMode() == FbxGeometryElement::eIndex ||
//				leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
//			{
//				int i;
//
//				pString += "           Indices: ";
//
//				int lIndexArrayCount = leMat->GetIndexArray().GetCount();
//				for (i = 0; i < lIndexArrayCount; i++)
//				{
//					pString += to_string(leMat->GetIndexArray().GetAt(i));
//
//					if (i < lIndexArrayCount - 1)
//					{
//						pString += ", ";
//					}
//				}
//
//				pString += "\n";
//
//				//FBXSDK_printf(lString);
//			}
//		}
//	}
//
//	//DisplayString("");
//	return pString;
//}


//string PrintTextureNames(FbxProperty &pProperty, FbxString& pConnectionString)
//{
//	int lLayeredTextureCount = pProperty.GetSrcObjectCount<FbxLayeredTexture>();
//	if (lLayeredTextureCount > 0)
//	{
//		for (int j = 0; j < lLayeredTextureCount; ++j)
//		{
//			FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
//			int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();
//			pConnectionString += " Texture ";
//
//			for (int k = 0; k < lNbTextures; ++k)
//			{
//				//lConnectionString += k;
//				pConnectionString += "\"";
//				pConnectionString += (char*)lLayeredTexture->GetName();
//				pConnectionString += "\"";
//				pConnectionString += " ";
//			}
//			pConnectionString += "of ";
//			pConnectionString += pProperty.GetName();
//			pConnectionString += " on layer ";
//			pConnectionString += j;
//		}
//		pConnectionString += " |";
//	}
//	else
//	{
//		//no layered texture simply get on the property
//		int lNbTextures = pProperty.GetSrcObjectCount<FbxTexture>();
//
//		if (lNbTextures > 0)
//		{
//			pConnectionString += " Texture ";
//			pConnectionString += " ";
//
//			for (int j = 0; j < lNbTextures; ++j)
//			{
//				FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
//				if (lTexture)
//				{
//					pConnectionString += "\"";
//					pConnectionString += (char*)lTexture->GetName();
//					pConnectionString += "\"";
//					pConnectionString += " ";
//				}
//			}
//			pConnectionString += "of ";
//			pConnectionString += pProperty.GetName();
//			pConnectionString += " |";
//		}
//	}
//}