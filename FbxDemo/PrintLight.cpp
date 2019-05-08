#include "PrintLight.h"

#include "Filenames.h"

#include <fstream>
#include <iostream>

/*
========================================================================================================================

	PrintLight prints the light information per light, including Light name, light type, if it casts light, colour, intensity, outer angle and fog values.

	// Martina Molid

========================================================================================================================
*/

void PrintLight(FbxNode* pNode, DirLight* dirlight, SpotLight* spotLight)
{
	FbxLight* lLight = (FbxLight*)pNode->GetNodeAttribute();

	DisplayString("Light Name: ", (char *)pNode->GetName());

	string tempName = (char *)pNode->GetName();
	int nameLength = strlen(tempName.c_str());
	char lightName[NAME_SIZE];
	for (int i = 0; i < nameLength; i++) {
		lightName[i] = tempName[i];
	}
	lightName[nameLength] = '\0'; // Puts a \0 at the end of the name, still printing out whitespace into the binary file
	//std::cout << "Light name: " << lightName << endl;

	//DisplayMetaDataConnections(lLight);

	const char* lLightTypes[] = { "Point", "Directional", "Spot", "Area", "Volume" };

	DisplayString("    Type: ", lLightTypes[lLight->LightType.Get()]);
	tempName = lLightTypes[lLight->LightType.Get()];
	nameLength = strlen(tempName.c_str());
	char lightType[NAME_SIZE];
	for (int i = 0; i < nameLength; i++) {
		lightType[i] = tempName[i];
	}
	lightType[nameLength] = '\0'; // Puts a \0 at the end of the name, still printing out whitespace into the binary file
	//std::cout << "Light type: " << lightType << endl;

	DisplayBool("    Cast Light: ", lLight->CastLight.Get());
	//lightInfo.castLight = lLight->CastLight.Get();

	if (!(lLight->FileName.Get().IsEmpty()))
	{
		// MM: Failsafe?
		DisplayString("    Gobo");

		DisplayString("        File Name: \"", lLight->FileName.Get().Buffer(), "\"");
		DisplayBool("        Ground Projection: ", lLight->DrawGroundProjection.Get());
		DisplayBool("        Volumetric Projection: ", lLight->DrawVolumetricLight.Get());
		DisplayBool("        Front Volumetric Projection: ", lLight->DrawFrontFacingVolumetricLight.Get());
	}

	DisplayString("    Default Animation Values");

	FbxDouble3 c = lLight->Color.Get();
	FbxColor lColor(c[0], c[1], c[2]);
	DisplayColor("        Default Color: ", lColor);
	//lightInfo.colour[0] = (float)c[0];
	//lightInfo.colour[1] = (float)c[1];
	//lightInfo.colour[2] = (float)c[2];

	DisplayDouble("        Default Intensity: ", lLight->Intensity.Get());
	//lightInfo.intensity = (float)lLight->Intensity.Get();
	DisplayDouble("        Default Outer Angle: ", lLight->OuterAngle.Get());
	//lightInfo.outAngle = (float)lLight->OuterAngle.Get();
	DisplayDouble("        Default Fog: ", lLight->Fog.Get());
	//lightInfo.fog = (float)lLight->Fog.Get();

}