#pragma once

#include "PrintInfo.h"
#include "PrintMaterial.h"
#include "Filenames.h"

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

void DisplayUserProperties(FbxObject* pObject, MeshHolder* mesh);
void GetMesh(FbxNode* pNode, MeshHolder* mesh, vector<PhongMaterial>& materials);