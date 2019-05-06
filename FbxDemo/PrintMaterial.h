#pragma once
#include <fbxsdk.h>

#include "PrintInfo.h"
#include "Filenames.h"


#include <iostream>
#include <fstream>
using namespace std;

void PrintMaterial(FbxGeometry* pGeometry, vector<PhongMaterial2>& mats, MeshHolder* mesh);