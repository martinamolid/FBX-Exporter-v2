#include "PrintMesh.h"
#include "DisplayCommon.h"

//#define MAT_HEADER_LENGTH 200

void GetPolygons(FbxMesh* pMesh, MeshHolder* mesh);


void GetMesh(FbxNode* pNode, MeshHolder* mesh, vector<PhongMaterial>& materials)
{
	FbxMesh* fbxMesh = (FbxMesh*)pNode->GetNodeAttribute();

	// Applies the mesh name
	int nameLength = (int)strlen(pNode->GetName());
	string nameBuffer = pNode->GetName();
	for (int j = 0; j < nameLength; j++)
		mesh->name[j] = nameBuffer[j];
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


	{
		int i, lControlPointsCount = fbxMesh->GetControlPointsCount();
		FbxVector4* lControlPoints = fbxMesh->GetControlPoints();

		DisplayString("    Control Points");

		for (i = 0; i < lControlPointsCount; i++)
		{
			DisplayInt("        Control Point ", i);
			Display3DVector("            Coordinates: ", lControlPoints[i]);

			for (int j = 0; j < fbxMesh->GetElementNormalCount(); j++)
			{
				FbxGeometryElementNormal* leNormals = fbxMesh->GetElementNormal(j);
				if (leNormals->GetMappingMode() == FbxGeometryElement::eByControlPoint)
				{
					char header[100];
					FBXSDK_sprintf(header, 100, "            Normal Vector: ");
					if (leNormals->GetReferenceMode() == FbxGeometryElement::eDirect)
						Display3DVector(header, leNormals->GetDirectArray().GetAt(i));
				}
			}
		}
	}





	// MM: Builds vertices for each polygon, and adds to the vertices array we allocated memory for
	int vertexId = 0;
	for (i = 0; i < lPolygonCount; i++)
	{
		int l;
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
						vertices[vertexId].uv[0] = (float)leUV->GetDirectArray().GetAt(lControlPointIndex)[0];
						vertices[vertexId].uv[1] = (float)leUV->GetDirectArray().GetAt(lControlPointIndex)[1];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leUV->GetIndexArray().GetAt(lControlPointIndex);
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
						vertices[vertexId].normal[0] = (float)leNormal->GetDirectArray().GetAt(vertexId)[0];
						vertices[vertexId].normal[1] = (float)leNormal->GetDirectArray().GetAt(vertexId)[1];
						vertices[vertexId].normal[2] = (float)leNormal->GetDirectArray().GetAt(vertexId)[2];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leNormal->GetIndexArray().GetAt(vertexId);
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
					if (leNormal->GetReferenceMode() == FbxGeometryElement::eDirect) 
					{
						//Display3DVector(header, leNormal->GetDirectArray().GetAt(lControlPointIndex));
		
						vertices[vertexId].normal[0] = (float)leNormal->GetDirectArray().GetAt(lControlPointIndex)[0];
						vertices[vertexId].normal[1] = (float)leNormal->GetDirectArray().GetAt(lControlPointIndex)[1];
						vertices[vertexId].normal[2] = (float)leNormal->GetDirectArray().GetAt(lControlPointIndex)[2];
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
						vertices[vertexId].tangent[0] = (float)leTangent->GetDirectArray().GetAt(vertexId)[0];
						vertices[vertexId].tangent[1] = (float)leTangent->GetDirectArray().GetAt(vertexId)[1];
						vertices[vertexId].tangent[2] = (float)leTangent->GetDirectArray().GetAt(vertexId)[2];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leTangent->GetIndexArray().GetAt(vertexId);
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
						vertices[vertexId].bitangent[0] = (float)leBinormal->GetDirectArray().GetAt(vertexId)[0];
						vertices[vertexId].bitangent[1] = (float)leBinormal->GetDirectArray().GetAt(vertexId)[1];
						vertices[vertexId].bitangent[2] = (float)leBinormal->GetDirectArray().GetAt(vertexId)[2];
						break;
					case FbxGeometryElement::eIndexToDirect:
					{
						int id = leBinormal->GetIndexArray().GetAt(vertexId);
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

