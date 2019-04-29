#include "PrintLight.h"

#include "Filenames.h"
#include "FormatName.h"

#include <fstream>
#include <iostream>

/*
========================================================================================================================

	PrintLight prints the light information per light, including Light name, light type, if it casts light, colour, intensity, outer angle and fog values.

	// Martina Molid

========================================================================================================================
*/

struct Light {
	//char name[NAME_SIZE];
	//char type[NAME_SIZE];
	bool castLight;
	float colour[3];
	float intensity;
	float outAngle;
	float fog;
};

string PrintLight(FbxNode* pNode)
{
	FbxLight* lLight = (FbxLight*)pNode->GetNodeAttribute();
	Light lightInfo;
	string pString;

	ofstream binFile(BINARY_FILE, ofstream::binary | ofstream::app);

	pString += PrintString("Light Name: ", (char *)pNode->GetName());

	string tempName = (char *)pNode->GetName();
	int nameLength = strlen(tempName.c_str());
	char lightName[NAME_SIZE];
	for (int i = 0; i < nameLength; i++) {
		lightName[i] = tempName[i];
	}
	lightName[nameLength] = '\0'; // Puts a \0 at the end of the name, still printing out whitespace into the binary file
	//std::cout << "Light name: " << lightName << endl;
	binFile.write((char*)lightName, sizeof(char) * NAME_SIZE);

	//DisplayMetaDataConnections(lLight);

	const char* lLightTypes[] = { "Point", "Directional", "Spot", "Area", "Volume" };

	pString += PrintString("    Type: ", lLightTypes[lLight->LightType.Get()]);
	tempName = lLightTypes[lLight->LightType.Get()];
	nameLength = strlen(tempName.c_str());
	char lightType[NAME_SIZE];
	for (int i = 0; i < nameLength; i++) {
		lightType[i] = tempName[i];
	}
	lightType[nameLength] = '\0'; // Puts a \0 at the end of the name, still printing out whitespace into the binary file
	//std::cout << "Light type: " << lightType << endl;
	binFile.write((char*)lightType, sizeof(char) * NAME_SIZE);

	pString += PrintBool("    Cast Light: ", lLight->CastLight.Get());
	lightInfo.castLight = lLight->CastLight.Get();

	if (!(lLight->FileName.Get().IsEmpty()))
	{
		// MM: Failsafe?
		pString += PrintString("    Gobo");

		pString += PrintString("        File Name: \"", lLight->FileName.Get().Buffer(), "\"");
		pString += PrintBool("        Ground Projection: ", lLight->DrawGroundProjection.Get());
		pString += PrintBool("        Volumetric Projection: ", lLight->DrawVolumetricLight.Get());
		pString += PrintBool("        Front Volumetric Projection: ", lLight->DrawFrontFacingVolumetricLight.Get());
	}

	//pString += PrintString("    Default Animation Values");

	FbxDouble3 c = lLight->Color.Get();
	FbxColor lColor(c[0], c[1], c[2]);
	pString += PrintColor("        Default Color: ", lColor);
	lightInfo.colour[0] = (float)c[0];
	lightInfo.colour[1] = (float)c[1];
	lightInfo.colour[2] = (float)c[2];

	pString += PrintDouble("        Default Intensity: ", lLight->Intensity.Get());
	lightInfo.intensity = (float)lLight->Intensity.Get();
	pString += PrintDouble("        Default Outer Angle: ", lLight->OuterAngle.Get());
	lightInfo.outAngle = (float)lLight->OuterAngle.Get();
	pString += PrintDouble("        Default Fog: ", lLight->Fog.Get());
	lightInfo.fog = (float)lLight->Fog.Get();

	binFile.write((char*)&lightInfo, sizeof(Light));

	binFile.close();
	return pString;
}