#pragma once

#include "PrintInfo.h"
#include "PrintMaterial.h"
#include "Filenames.h"

#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

string PrintMesh(FbxNode* pNode);

void DisplayUserProperties(FbxObject* pObject, MeshHolder* mesh);
void GetMesh(FbxNode* pNode, MeshHolder* mesh, vector<PhongMaterial2>& materials);