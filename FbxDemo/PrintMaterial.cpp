#include "PrintMaterial.h"

/*
========================================================================================================================

	PrintMaterial prints out the material information per mesh. 
	For each material connected to the mesh, the function prints material index, material name and the Phong attributes, nr of textures for the material.

	At the end of the loop, calls PrintTexture to print all the textures related to this material.
	
	Currently only supports Phong materials, but code for Lambert is left in case it would be desired.
	
	Material count is printed in PrintMesh->PrintPolygons instead of here in order to print at the top of the mesh.

	// Martina Molid

========================================================================================================================
*/

void PrintMaterial(FbxGeometry* pGeometry, vector<PhongMaterial2>& mats, MeshHolder* mesh)
{
	int materialCount = 0;
	FbxNode* materialNode = NULL;

	PhongMaterial2 *materials = nullptr;

	if (pGeometry) 
	{
		materialNode = pGeometry->GetNode();
		if (materialNode) 
		{
			materialCount = materialNode->GetMaterialCount();

			// MM: Allocates memory for the materials array, based on how many materials are to be read
			materials = new PhongMaterial2[materialCount];
		}
	}

	if (materialCount > 0)
	{
		FbxPropertyT<FbxDouble3> lKFbxDouble3;
		FbxPropertyT<FbxDouble> lKFbxDouble1;
		FbxColor theColor;

		for (int matIndex = 0; matIndex < materialCount; matIndex++)
		{
			FbxSurfaceMaterial *lMaterial = materialNode->GetMaterial(matIndex);

			// Applies the material name
			int nameLength = (int)strlen(lMaterial->GetName());
			string nameBuffer = lMaterial->GetName();
			for (int j = 0; j < nameLength; j++)
			{
				materials[matIndex].name[j] = nameBuffer[j];
			}
			materials[matIndex].name[nameLength] = '\0';
			// Puts a \0 at the end of the mesh name, still printing out whitespace into the binary file

			if (lMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
			{
				// We found a Phong material.  Display its properties.

				// Print the Ambient Color
				lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Ambient;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);

				materials[matIndex].ambient[0] = (float)lKFbxDouble3.Get()[0];
				materials[matIndex].ambient[1] = (float)lKFbxDouble3.Get()[1];
				materials[matIndex].ambient[2] = (float)lKFbxDouble3.Get()[2];

				// Print the Diffuse Color
				lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Diffuse;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);

				materials[matIndex].diffuse[0] = (float)lKFbxDouble3.Get()[0];
				materials[matIndex].diffuse[1] = (float)lKFbxDouble3.Get()[1];
				materials[matIndex].diffuse[2] = (float)lKFbxDouble3.Get()[2];

				// Print the Specular Color (unique to Phong materials)
				lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Specular;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);

				materials[matIndex].specular[0] = (float)lKFbxDouble3.Get()[0];
				materials[matIndex].specular[1] = (float)lKFbxDouble3.Get()[1];
				materials[matIndex].specular[2] = (float)lKFbxDouble3.Get()[2];

				// Print the Emissive Color
				lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Emissive;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);

				materials[matIndex].emissive[0] = (float)lKFbxDouble3.Get()[0];
				materials[matIndex].emissive[1] = (float)lKFbxDouble3.Get()[1];
				materials[matIndex].emissive[2] = (float)lKFbxDouble3.Get()[2];

				//Opacity is Transparency factor now
				lKFbxDouble1 = ((FbxSurfacePhong *)lMaterial)->TransparencyFactor;
				materials[matIndex].opacity = (float)lKFbxDouble1.Get();

				// -- GETTING THE NUMBER OF TEXTURES FOR THE MATERIAL --
				FbxProperty lProperty;
				int lTextureCount = 0;
				int nrOfTextures = 0;
				//int run = 0;
				int lTextureIndex;
				FBXSDK_FOR_EACH_TEXTURE(lTextureIndex)
				{
					lProperty = lMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[lTextureIndex]);
					//run++;
					lTextureCount = lProperty.GetSrcObjectCount<FbxTexture>();
					for (int j = 0; j < lTextureCount; j++) {
						FbxTexture* lTexture = lProperty.GetSrcObject<FbxTexture>(j);
						if (lTexture)
						{
							nrOfTextures++;
						}
					}
				}
				//cout << "Run: " << run << endl;

			}
			else
				cout << PrintString("Unknown type of Material") << endl;

		}
	}


	// Checks if the material already exists in the vector
	bool nameExists = false;
	for (int i = 0; i < materialCount; i++)
	{
		for (int j = 0; j < mats.size(); j++)
		{
			if ((string)mats[j].name == (string)materials[i].name)
			{
				nameExists = true;
			}
		}

		if (!nameExists)
		{
			mats.push_back(materials[i]);
			for (int j = 0; j < strlen(materials[i].name); j++)
			{
				mesh->materialName[j] = materials[i].name[j];
				mesh->materialName[strlen(materials[i].name)] = '\0';
			}
		}
		else
		{
			for (int j = 0; j < strlen(materials[i].name); j++)
			{
				mesh->materialName[j] = materials[i].name[j];
				mesh->materialName[strlen(materials[i].name)] = '\0';
			}
			nameExists = false;
		}
	}

	// Special case for the first material
	if (materialCount && mats.size() == 0)
	{
		mats.push_back(materials[0]);
		for (int j = 0; j < strlen(materials[0].name); j++)
		{
			mesh->materialName[j] = materials[0].name[j];
			mesh->materialName[strlen(materials[0].name)] = '\0';
		}
	}
		


	if (materials)
	{
		delete[] materials;
	}
}


string PrintMaterial(FbxGeometry* pGeometry)
{
	int lMaterialCount = 0;
	FbxNode* lNode = NULL;

	PhongMaterial *materials = nullptr;
	string pString;

	if (pGeometry) {
		lNode = pGeometry->GetNode();
		if (lNode) {
			lMaterialCount = lNode->GetMaterialCount();
			///pString += "Material Count: " + to_string(lMaterialCount) + "\n";
			/// MM: Printing how many materials exist for the mesh
			///binFile.write((char*)&lMaterialCount, sizeof(int));
			// MM: material count is printed in PrintMesh->PrintPolygons instead of here in order to print at the top of the mesh
			// MM: Allocates memory for the materials array, based on how many materials are to be read
			materials = new PhongMaterial[lMaterialCount];
		}
			
	}

	if (lMaterialCount > 0)
	{
		FbxPropertyT<FbxDouble3> lKFbxDouble3;
		FbxPropertyT<FbxDouble> lKFbxDouble1;
		FbxColor theColor;

		for (int lCount = 0; lCount < lMaterialCount; lCount++)
		{
			// MM: Binary file has to be opened inside the for-loop to make everything print correctly when we call PrintTexture later
			ofstream binFile(BINARY_FILE, ofstream::binary | ofstream::app);
			pString += PrintInt("        Material ", lCount);
			// MM: Printing the material index
			binFile.write((char*)&lCount, sizeof(int));

			FbxSurfaceMaterial *lMaterial = lNode->GetMaterial(lCount);

			
			pString += PrintString("            Name: \"", (char *)lMaterial->GetName(), "\"");
			// MM: Printing the material name
			string tempMatName = lMaterial->GetName();
			int nameLength = (int)strlen(tempMatName.c_str());
			char matName[NAME_SIZE]; 
			for (int i = 0; i < nameLength; i++) {
				matName[i] = tempMatName[i];
			}
			matName[nameLength] = '\0'; // Puts a \0 at the end of the material name, still printing out whitespace into the binary file
			cout << "Material name: " << matName << endl;

			binFile.write((char*)matName, sizeof(char) * NAME_SIZE);


			//Get the implementation to see if it's a hardware shader.
			const FbxImplementation* lImplementation = GetImplementation(lMaterial, FBXSDK_IMPLEMENTATION_HLSL);
			FbxString lImplemenationType = "HLSL";
			if (!lImplementation)
			{
				lImplementation = GetImplementation(lMaterial, FBXSDK_IMPLEMENTATION_CGFX);
				lImplemenationType = "CGFX";
			}
			/*//if (lImplementation)
			//{
			//	//Now we have a hardware shader, let's read it
			//	FBXSDK_printf("            Hardware Shader Type: %s\n", lImplemenationType.Buffer());
			//	const FbxBindingTable* lRootTable = lImplementation->GetRootTable();
			//	FbxString lFileName = lRootTable->DescAbsoluteURL.Get();
			//	FbxString lTechniqueName = lRootTable->DescTAG.Get();


			//	const FbxBindingTable* lTable = lImplementation->GetRootTable();
			//	size_t lEntryNum = lTable->GetEntryCount();

			//	for (int i = 0; i < (int)lEntryNum; ++i)
			//	{
			//		const FbxBindingTableEntry& lEntry = lTable->GetEntry(i);
			//		const char* lEntrySrcType = lEntry.GetEntryType(true);
			//		FbxProperty lFbxProp;


			//		FbxString lTest = lEntry.GetSource();
			//		FBXSDK_printf("            Entry: %s\n", lTest.Buffer());


			//		if (strcmp(FbxPropertyEntryView::sEntryType, lEntrySrcType) == 0)
			//		{
			//			lFbxProp = lMaterial->FindPropertyHierarchical(lEntry.GetSource());
			//			if (!lFbxProp.IsValid())
			//			{
			//				lFbxProp = lMaterial->RootProperty.FindHierarchical(lEntry.GetSource());
			//			}


			//		}
			//		else if (strcmp(FbxConstantEntryView::sEntryType, lEntrySrcType) == 0)
			//		{
			//			lFbxProp = lImplementation->GetConstants().FindHierarchical(lEntry.GetSource());
			//		}
			//		if (lFbxProp.IsValid())
			//		{
			//			if (lFbxProp.GetSrcObjectCount<FbxTexture>() > 0)
			//			{
			//				//do what you want with the textures
			//				for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxFileTexture>(); ++j)
			//				{
			//					FbxFileTexture *lTex = lFbxProp.GetSrcObject<FbxFileTexture>(j);
			//					FBXSDK_printf("           File Texture: %s\n", lTex->GetFileName());
			//				}
			//				for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxLayeredTexture>(); ++j)
			//				{
			//					FbxLayeredTexture *lTex = lFbxProp.GetSrcObject<FbxLayeredTexture>(j);
			//					FBXSDK_printf("        Layered Texture: %s\n", lTex->GetName());
			//				}
			//				for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxProceduralTexture>(); ++j)
			//				{
			//					FbxProceduralTexture *lTex = lFbxProp.GetSrcObject<FbxProceduralTexture>(j);
			//					FBXSDK_printf("     Procedural Texture: %s\n", lTex->GetName());
			//				}
			//			}
			//			else
			//			{
			//				FbxDataType lFbxType = lFbxProp.GetPropertyDataType();
			//				FbxString blah = lFbxType.GetName();
			//				if (FbxBoolDT == lFbxType)
			//				{
			//					pString += PrintBool("                Bool: ", lFbxProp.Get<FbxBool>());
			//				}
			//				else if (FbxIntDT == lFbxType || FbxEnumDT == lFbxType)
			//				{
			//					pString += PrintInt("                Int: ", lFbxProp.Get<FbxInt>());
			//				}
			//				else if (FbxFloatDT == lFbxType)
			//				{
			//					pString += PrintDouble("                Float: ", lFbxProp.Get<FbxFloat>());

			//				}
			//				else if (FbxDoubleDT == lFbxType)
			//				{
			//					pString += PrintDouble("                Double: ", lFbxProp.Get<FbxDouble>());
			//				}
			//				else if (FbxStringDT == lFbxType
			//					|| FbxUrlDT == lFbxType
			//					|| FbxXRefUrlDT == lFbxType)
			//				{
			//					pString += PrintString("                String: ", lFbxProp.Get<FbxString>().Buffer());
			//				}
			//				else if (FbxDouble2DT == lFbxType)
			//				{
			//					FbxDouble2 lDouble2 = lFbxProp.Get<FbxDouble2>();
			//					FbxVector2 lVect;
			//					lVect[0] = lDouble2[0];
			//					lVect[1] = lDouble2[1];

			//					pString += Print2DVector("                2D vector: ", lVect);
			//				}
			//				else if (FbxDouble3DT == lFbxType || FbxColor3DT == lFbxType)
			//				{
			//					FbxDouble3 lDouble3 = lFbxProp.Get<FbxDouble3>();


			//					FbxVector4 lVect;
			//					lVect[0] = lDouble3[0];
			//					lVect[1] = lDouble3[1];
			//					lVect[2] = lDouble3[2];
			//					pString += Print3DVector("                3D vector: ", lVect);
			//				}

			//				else if (FbxDouble4DT == lFbxType || FbxColor4DT == lFbxType)
			//				{
			//					FbxDouble4 lDouble4 = lFbxProp.Get<FbxDouble4>();
			//					FbxVector4 lVect;
			//					lVect[0] = lDouble4[0];
			//					lVect[1] = lDouble4[1];
			//					lVect[2] = lDouble4[2];
			//					lVect[3] = lDouble4[3];
			//					pString += Print4DVector("                4D vector: ", lVect);
			//				}
			//				else if (FbxDouble4x4DT == lFbxType)
			//				{
			//					FbxDouble4x4 lDouble44 = lFbxProp.Get<FbxDouble4x4>();
			//					for (int j = 0; j < 4; ++j)
			//					{

			//						FbxVector4 lVect;
			//						lVect[0] = lDouble44[j][0];
			//						lVect[1] = lDouble44[j][1];
			//						lVect[2] = lDouble44[j][2];
			//						lVect[3] = lDouble44[j][3];
			//						pString += Print4DVector("                4x4D vector: ", lVect);
			//					}

			//				}
			//			}

			//		}
			//	}
			//}*/ // MM: next line needs to be 'else if' in case this^^ is needed again later
			if (lMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
			{
				// We found a Phong material.  Display its properties.

				// Print the Ambient Color
				lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Ambient;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				pString += PrintColor("            Ambient: ", theColor);
				materials[lCount].ambient[0] = (float)lKFbxDouble3.Get()[0]; 
				materials[lCount].ambient[1] = (float)lKFbxDouble3.Get()[1];
				materials[lCount].ambient[2] = (float)lKFbxDouble3.Get()[2];

				// Print the Diffuse Color
				lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Diffuse;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				pString += PrintColor("            Diffuse: ", theColor);
				materials[lCount].diffuse[0] = (float)lKFbxDouble3.Get()[0];
				materials[lCount].diffuse[1] = (float)lKFbxDouble3.Get()[1];
				materials[lCount].diffuse[2] = (float)lKFbxDouble3.Get()[2];

				// Print the Specular Color (unique to Phong materials)
				lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Specular;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				pString += PrintColor("            Specular: ", theColor);
				materials[lCount].specular[0] = (float)lKFbxDouble3.Get()[0];
				materials[lCount].specular[1] = (float)lKFbxDouble3.Get()[1];
				materials[lCount].specular[2] = (float)lKFbxDouble3.Get()[2];

				// Print the Emissive Color
				lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Emissive;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				pString += PrintColor("            Emissive: ", theColor);
				materials[lCount].emissive[0] = (float)lKFbxDouble3.Get()[0];
				materials[lCount].emissive[1] = (float)lKFbxDouble3.Get()[1];
				materials[lCount].emissive[2] = (float)lKFbxDouble3.Get()[2];

				//Opacity is Transparency factor now
				lKFbxDouble1 = ((FbxSurfacePhong *)lMaterial)->TransparencyFactor;
				pString += PrintDouble("            Opacity: ", 1.0 - lKFbxDouble1.Get());
				materials[lCount].opacity = (float)lKFbxDouble1.Get();

				// Print the Shininess
				lKFbxDouble1 = ((FbxSurfacePhong *)lMaterial)->Shininess;
				pString += PrintDouble("            Shininess: ", lKFbxDouble1.Get());
				materials[lCount].shininess = (float)lKFbxDouble1.Get();

				// Print the Reflectivity
				lKFbxDouble1 = ((FbxSurfacePhong *)lMaterial)->ReflectionFactor;
				pString += PrintDouble("            Reflectivity: ", lKFbxDouble1.Get());
				materials[lCount].reflectivity = (float)lKFbxDouble1.Get();


				// -- GETTING THE NUMBER OF TEXTURES FOR THE MATERIAL --
				FbxProperty lProperty;
				int lTextureCount = 0;
				int nrOfTextures = 0;
				//int run = 0;
				int lTextureIndex;
				FBXSDK_FOR_EACH_TEXTURE(lTextureIndex)
				{
					lProperty = lMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[lTextureIndex]);
					//run++;
					lTextureCount = lProperty.GetSrcObjectCount<FbxTexture>();
					for (int j = 0; j < lTextureCount; j++) {
						FbxTexture* lTexture = lProperty.GetSrcObject<FbxTexture>(j);
						if (lTexture)
						{
							nrOfTextures++;
						}	
					}
				}
				//cout << "Run: " << run << endl;

				pString += "            Nr of Textures: " + to_string(nrOfTextures) + "\n";
				materials[lCount].nrOfTextures = nrOfTextures;

				// -- Writing the material to the binary file
				binFile.write((char*)&materials[lCount], sizeof(PhongMaterial));
				// MM: File has to be closed here as it is reopened in PrintTexture
				binFile.close();
				// MM: PrintTexture is called here, printing out texture information for all textures per material
				pString += PrintTexture(lMaterial, lCount);

			}
			else if (lMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
			{
				// We found a Lambert material. Display its properties.
				// Display the Ambient Color
				lKFbxDouble3 = ((FbxSurfaceLambert *)lMaterial)->Ambient;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				pString += PrintColor("            Ambient: ", theColor);
				/*float amb[3] = { (float)lKFbxDouble3.Get()[0], (float)lKFbxDouble3.Get()[1], (float)lKFbxDouble3.Get()[2] };
				binFile.write((char*)&amb[0], sizeof(float));
				binFile.write((char*)&amb[1], sizeof(float));
				binFile.write((char*)&amb[2], sizeof(float));*/

				// Display the Diffuse Color
				lKFbxDouble3 = ((FbxSurfaceLambert *)lMaterial)->Diffuse;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				pString += PrintColor("            Diffuse: ", theColor);
				/*float diff[3] = { (float)lKFbxDouble3.Get()[0], (float)lKFbxDouble3.Get()[1], (float)lKFbxDouble3.Get()[2] };
				binFile.write((char*)&diff[0], sizeof(float));
				binFile.write((char*)&diff[1], sizeof(float));
				binFile.write((char*)&diff[2], sizeof(float));*/

				// Display the Emissive
				lKFbxDouble3 = ((FbxSurfaceLambert *)lMaterial)->Emissive;
				theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
				pString += PrintColor("            Emissive: ", theColor);
				/*float emis[3] = { (float)lKFbxDouble3.Get()[0], (float)lKFbxDouble3.Get()[1], (float)lKFbxDouble3.Get()[2] };
				binFile.write((char*)&emis[0], sizeof(float));
				binFile.write((char*)&emis[1], sizeof(float));
				binFile.write((char*)&emis[2], sizeof(float));*/

				// Display the Opacity
				lKFbxDouble1 = ((FbxSurfaceLambert *)lMaterial)->TransparencyFactor;
				pString += PrintDouble("            Opacity: ", 1.0 - lKFbxDouble1.Get());
				/*float opac = (float)lKFbxDouble1.Get();
				binFile.write((char*)&opac, sizeof(float));*/
			}
			else
				pString += PrintString("Unknown type of Material");


			// MM: Use this (but move it further up) if we want to know which shading model to use
			//FbxPropertyT<FbxString> lString;
			//lString = lMaterial->ShadingModel;
			//pString += PrintString("            Shading Model: ", lString.Get().Buffer());
			//pString += PrintString("");
		}
	}

	if (materials != nullptr) {
		delete[] materials;
	}

	return pString;
}