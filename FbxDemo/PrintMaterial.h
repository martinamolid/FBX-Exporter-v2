#pragma once
#include <fbxsdk.h>

#include "PrintInfo.h"
#include "Filenames.h"
#include "PrintTexture.h"

#include <iostream>
#include <fstream>
using namespace std;

string PrintMaterial(FbxGeometry* pGeometry);
void PrintMaterial(FbxGeometry* pGeometry, vector<PhongMaterial2>& mats, Mesh* mesh);