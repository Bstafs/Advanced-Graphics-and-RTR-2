#pragma once

#define STB_IMAGE_IMPLEMENTATION

#include <windows.h>
#include <windowsx.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <DirectXCollision.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include <iostream>
#include  <fstream>

#include "DrawableGameObject.h"
#include "structures.h"

#include <vector>

#include <algorithm>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

#include "Camera.h"

#include "Terrain.h"

#include "Skeleton.h"

#include "stb_image.h"

#include <noise/noise.h>
#include  "noiseutils.h"

using namespace std;
using namespace noise;

typedef vector<DrawableGameObject*> vecDrawables;



const int terrainSizeHeight = 65;
const int terrainSizeWidth = 65;

int roughness = 5;

int terrainMap[terrainSizeWidth][terrainSizeHeight];